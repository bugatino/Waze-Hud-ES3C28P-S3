#define main previewHarnessMain
#include "../tools/host_preview/preview.cpp"
#undef main
#include "protocol/hlp_decoder.h"
#include "display/colors.h"

int laneInk(){
    using namespace waze_hud;int count=0;
    for(int y=209;y<243;++y)for(int x=20;x<220;++x){
        const auto p=g_fb[y*240+x];if(p==colors::White||p==colors::Cyan)++count;
    }return count;
}
int main(){
    using namespace waze_hud;HlpDecoder decoder;HudState state;
    DeviceSettings config;config.portraitDisplay=true;
    const char *frames[]={
        "{\"ts\":1,\"nav\":1,\"trn\":1,\"dst\":518,\"lan\":[[5,1],[5,1],[33,1],[32,0]]}",
        "{\"ts\":2,\"nav\":0,\"lan\":[[5,1],[5,1],[33,1],[32,0]]}",
        "{\"ts\":3,\"nav\":true,\"lan\":[[4,1],[1,1],[33,1],[32,0]]}"
    };
    for(int i=0;i<3;++i){
        auto *frame=cJSON_Parse(frames[i]);assert(decoder.decodeState(frame,state));cJSON_Delete(frame);
        assert(state.laneCount==4&&state.laneWireCount==4);
        if(i==2){assert(state.navigationActive&&state.laneSelectionInvalid&&state.lanes[0].directionMask==4&&state.lanes[0].selectedMask==0);}
        char name[32];std::snprintf(name,sizeof(name),"v4_lane_wire_%d",i);shot(name,state,config);assert(laneInk()>150);
    }
    auto *empty=cJSON_Parse("{\"ts\":4,\"nav\":1,\"lan\":[]}");assert(decoder.decodeState(empty,state));cJSON_Delete(empty);
    assert(state.laneCount==0&&state.laneWireCount==0);shot("v4_lane_empty_clears",state,config);assert(laneInk()==0);
    puts("Real JSON decoder -> real striped renderer: nav integer/boolean/off, bad selection preserves directions, explicit empty clears lanes passed");
}
