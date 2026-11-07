#include "config/device_config.h"
#include "state/hud_state_store.h"
#include "nvs.h"
#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <cstdio>
std::map<std::string,uint32_t> disk;
bool failSave=false;
esp_err_t nvs_open(const char*,int mode,int *n){*n=1;return mode==NVS_READONLY&&disk.empty()?ESP_ERR_NVS_NOT_FOUND:ESP_OK;}
void nvs_close(int){}
esp_err_t nvs_commit(int){return failSave?ESP_FAIL:ESP_OK;}
#define ACCESS(T,S) esp_err_t nvs_get_##S(int,const char*k,T*v){if(!disk.count(k))return ESP_ERR_NVS_NOT_FOUND;*v=disk[k];return ESP_OK;} esp_err_t nvs_set_##S(int,const char*k,T v){disk[k]=v;return ESP_OK;}
ACCESS(uint8_t,u8) ACCESS(int8_t,i8) ACCESS(uint32_t,u32)
void taskENTER_CRITICAL(portMUX_TYPE*){} void taskEXIT_CRITICAL(portMUX_TYPE*){}
const char *esp_err_to_name(int){return "test";}
namespace waze_hud {
HudStateStore &HudStateStore::instance(){static HudStateStore s;return s;}
void HudStateStore::refresh(){}
}
std::vector<std::string> lines;
void collect(const char *s,void*){lines.emplace_back(s);}
void send(cJSON *m){waze_hud::DeviceConfig::instance().handleMessage(m,collect,nullptr);cJSON_Delete(m);}
cJSON *message(const char*t){auto *m=cJSON_CreateObject();cJSON_AddStringToObject(m,"t",t);return m;}
std::vector<std::string> schema(){lines.clear();waze_hud::DeviceConfig::instance().publishSchema(collect,nullptr);return lines;}
void begin(int tx,int rev,int count=12){auto*m=message("cfg_set_begin");cJSON_AddNumberToObject(m,"tx",tx);cJSON_AddNumberToObject(m,"rev",rev);cJSON_AddNumberToObject(m,"count",count);send(m);}
void fields(int tx,const std::vector<std::string>&s,int volume=0){for(const auto&line:s){auto*i=cJSON_Parse(line.c_str());auto*t=cJSON_GetObjectItemCaseSensitive(i,"t");if(std::string(t->valuestring)=="cfg_item"){
 auto*m=message("cfg_set");cJSON_AddNumberToObject(m,"tx",tx);auto*id=cJSON_GetObjectItemCaseSensitive(i,"id");cJSON_AddStringToObject(m,"id",id->valuestring);
 if(std::string(id->valuestring)=="portrait_display")cJSON_AddBoolToObject(m,"value",true);
 else if(std::string(id->valuestring)=="volume")cJSON_AddNumberToObject(m,"value",volume);
 else if(std::string(id->valuestring)=="hud_audio")cJSON_AddBoolToObject(m,"value",true);
 else cJSON_AddItemToObject(m,"value",cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(i,"value"),true));send(m);
 }cJSON_Delete(i);}}
void commit(int tx){auto*m=message("cfg_set_commit");cJSON_AddNumberToObject(m,"tx",tx);send(m);}
bool ackOk(){auto*m=cJSON_Parse(lines.back().c_str());bool ok=cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(m,"ok"));cJSON_Delete(m);return ok;}
int main(){using namespace waze_hud;auto &c=DeviceConfig::instance();assert(c.init()==ESP_OK);assert(!c.snapshot().hudAudioEnabled);auto s=schema();assert(s.size()==14);
 auto *b=cJSON_Parse(s[0].c_str());assert(cJSON_GetObjectItemCaseSensitive(b,"count")->valueint==12);cJSON_Delete(b);
 begin(1,c.snapshot().revision);fields(1,s);commit(1);assert(ackOk());assert(c.snapshot().portraitDisplay&&c.snapshot().volume==0&&c.snapshot().hudAudioEnabled);assert(disk["portrait"]==1&&disk["volume"]==0&&disk["hud_audio"]==1);
 s=schema();begin(2,c.snapshot().revision);fields(2,s,50);c.setVolume(5,false);commit(2);assert(!ackOk());assert(c.snapshot().volume==5);
 c.persistVolume();assert(disk["volume"]==5);assert(c.init()==ESP_OK);assert(c.snapshot().volume==5&&c.snapshot().portraitDisplay);
 s=schema();begin(3,c.snapshot().revision);fields(3,s,101);assert(!ackOk());commit(3);assert(c.snapshot().volume==5);
 begin(4,c.snapshot().revision,9);assert(!ackOk());
 const auto rev=c.snapshot().revision;failSave=true;assert(c.setVolume(80)==ESP_FAIL);assert(c.snapshot().revision==rev&&c.snapshot().volume==5);failSave=false;
 disk.clear();disk["schema_ver"]=7;disk["revision"]=7;disk["rotate"]=1;disk["mirror"]=1;disk["brightness"]=50;
 assert(c.init()==ESP_OK);assert(c.snapshot().rotateDisplay&&c.snapshot().mirrorHud&&c.snapshot().brightness==50);
 assert(!c.snapshot().portraitDisplay&&c.snapshot().volume==50&&c.snapshot().revision==9&&disk["schema_ver"]==9&&!c.snapshot().hudAudioEnabled);
 disk.clear();disk["schema_ver"]=8;disk["revision"]=8;disk["portrait"]=1;disk["volume"]=25;disk["mirror"]=1;
 assert(c.init()==ESP_OK);assert(c.snapshot().portraitDisplay&&c.snapshot().mirrorHud&&c.snapshot().volume==25&&!c.snapshot().hudAudioEnabled);
 puts("Schema/migration, portrait/mute persistence, validation, concurrent-change rejection and NVS failure passed");}
