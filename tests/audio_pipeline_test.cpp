// Runs the real overspeed audio task against fake GPIO/I2S/codec endpoints.
#include "audio/overspeed_audio.h"
#include "audio/overspeed_voice.h"
#include "audio/pcm_gain.h"
#include "audio/speech_bank.h"
#include "config/device_config.h"
#include "state/hud_state_store.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "es8311.h"
#include <cassert>
#include <cstdio>

namespace {
waze_hud::DeviceSettings config;
waze_hud::HudState state;
void (*worker)(void *)=nullptr;
TickType_t ticks=0;
int amp=1,writes=0,nonzero=0,firstWriteTick=-1,overspeedTick=-1,muteAt=-1;
bool changeDuringPlayback=false,sawMuted=false,sawResumed=false;
struct Finished {};
}
namespace waze_hud {
#define PCM8 -20000,20000,-10000,10000,-30000,30000,-1,1
#define PCM64 PCM8,PCM8,PCM8,PCM8,PCM8,PCM8,PCM8,PCM8
#define PCM512 PCM64,PCM64,PCM64,PCM64,PCM64,PCM64,PCM64,PCM64
extern const int16_t kOverspeedVoice[]={PCM512,PCM512,PCM512,PCM512};
extern const std::size_t kOverspeedVoiceSamples=sizeof(kOverspeedVoice)/sizeof(int16_t);
SpeechClip speechClip(Prompt){static const uint8_t data[256]={0,128,64,192};return {data,sizeof(data)};}
DeviceConfig &DeviceConfig::instance(){static DeviceConfig c;return c;}
DeviceSettings DeviceConfig::snapshot()const{return config;}
HudStateStore &HudStateStore::instance(){static HudStateStore s;return s;}
HudState HudStateStore::snapshot()const{return state;}
esp_err_t initSharedI2c(){return ESP_OK;}
}
esp_err_t gpio_config(const gpio_config_t *p){assert(p->pin_bit_mask==(1ULL<<1));return ESP_OK;}
esp_err_t gpio_set_level(gpio_num_t pin,int value){assert(pin==1);amp=value;if(changeDuringPlayback&&config.volume==0&&writes>0&&value==1)sawMuted=true;return ESP_OK;}
esp_err_t i2s_new_channel(const i2s_chan_config_t *p,i2s_chan_handle_t *h,void *){assert(p->auto_clear);*h=reinterpret_cast<void *>(1);return ESP_OK;}
esp_err_t i2s_channel_init_std_mode(i2s_chan_handle_t,const i2s_std_config_t *p){
 assert(p->clk_cfg.rate==16000&&p->slot_cfg.width==16&&p->slot_cfg.mode==2);
 assert(p->gpio_cfg.mclk==4&&p->gpio_cfg.bclk==5&&p->gpio_cfg.ws==7&&p->gpio_cfg.dout==8);return ESP_OK;
}
esp_err_t i2s_channel_enable(i2s_chan_handle_t){return ESP_OK;}
esp_err_t i2s_channel_write(i2s_chan_handle_t,const void *buffer,std::size_t bytes,std::size_t *written,TickType_t){
 if(firstWriteTick<0)firstWriteTick=ticks;
 const auto *samples=static_cast<const int16_t *>(buffer);bool audible=false;
 for(std::size_t i=0;i<bytes/sizeof(int16_t);i+=2){
   assert(samples[i]==samples[i+1]);
   if(config.volume==0)assert(samples[i]==0);
   audible|=samples[i]!=0;
 }
 assert(amp==(config.volume==0?1:0));
 if(audible){++nonzero;if(samples[0]==waze_hud::scalePcm(-17000,config.volume)&&overspeedTick<0)overspeedTick=ticks;}
 if(changeDuringPlayback){
   if(writes==2){config.volume=0;muteAt=ticks;}
   if(sawMuted&&config.volume==20&&audible&&amp==0)sawResumed=true;
 }
 ++writes;*written=bytes;return ESP_OK;
}
es8311_handle_t es8311_create(i2c_port_t port,int address){assert(port==0&&address==0x18);return reinterpret_cast<void *>(1);}
esp_err_t es8311_init(es8311_handle_t,const es8311_clock_config_t *p,int a,int b){assert(p->sample_frequency==16000&&p->mclk_frequency==4096000&&a==16&&b==16);return ESP_OK;}
void es8311_delete(es8311_handle_t){}
esp_err_t es8311_sample_frequency_config(es8311_handle_t,int a,int b){assert(a==4096000&&b==16000);return ESP_OK;}
esp_err_t es8311_voice_volume_set(es8311_handle_t,int volume,int *){assert(volume==75);return ESP_OK;}
TickType_t xTaskGetTickCount(){return ticks;}
int64_t esp_timer_get_time(){return int64_t(ticks)*1000;}
void vTaskDelay(TickType_t n){ticks+=n;if(changeDuringPlayback&&muteAt>=0&&ticks>=unsigned(muteAt+400))config.volume=20;if(n==200&&ticks>7000)throw Finished{};}
BaseType_t xTaskCreate(void (*fn)(void *),const char *,int,void *,int,void *){worker=fn;return pdPASS;}

void run(unsigned volume,bool stale=false,bool change=false,int speed=71){
 config={};config.volume=volume;
 state={};state.connected=true;state.hasProducerState=true;state.signalStale=stale;state.speedKmh=speed;state.speedLimitKmh=60;
 ticks=writes=nonzero=0;firstWriteTick=overspeedTick=muteAt=-1;changeDuringPlayback=change;sawMuted=sawResumed=false;
 assert(waze_hud::startOverspeedAudio()==ESP_OK&&worker);
 try{worker(nullptr);assert(false);}catch(const Finished&){}
 assert(amp==1);
 if(volume==0||stale)assert(writes==0);
 else {assert(nonzero>0&&firstWriteTick>=0);if(speed>60)assert(overspeedTick>=1500);else assert(overspeedTick<0);if(change)assert(sawMuted&&sawResumed);}
}
int main(){run(35);run(0);run(35,true);run(35,false,false,60);run(35,false,true);
 puts("Real audio task: original S3 pins/clocks, local speech, 35% stereo output, zero mute, cancelled-clip resume, stale silence and debounced overspeed passed");}
