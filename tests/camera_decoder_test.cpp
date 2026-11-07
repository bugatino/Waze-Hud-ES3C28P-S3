#include "protocol/hlp_decoder.h"
#include <cassert>
#include <cstdio>
int64_t esp_timer_get_time(){return 1000000;}

int main(){
 using namespace waze_hud;
 HlpDecoder decoder;HudState state;
 // SDK wire code 40 remains 40, with its documented enforcement meaning.
 auto *frame=cJSON_Parse("{\"ts\":1,\"alr\":40,\"alrD\":300,\"lan\":[[5,1],[5,1],[33,1],[32,0]]}");
 assert(decoder.decodeState(frame,state));cJSON_Delete(frame);
 assert(state.nearestAlert.kind==AlertKind::EnforcementCamera&&state.nearestAlert.distanceM==300);
 assert(static_cast<int>(state.nearestAlert.kind)==40);
 assert(state.laneCount==4&&state.lanes[0].directionMask==5&&state.lanes[2].directionMask==33&&state.lanes[3].selectedMask==0);
 frame=cJSON_Parse("{\"ts\":2,\"alrs\":[{\"k\":40,\"d\":100},{\"k\":2,\"d\":200},{\"k\":3,\"d\":300},{\"k\":42,\"d\":400}]}");
 assert(decoder.decodeState(frame,state));cJSON_Delete(frame);
 assert(state.nearestAlert.kind==AlertKind::EnforcementCamera);
 assert(state.upcomingAlertCount==3&&state.upcomingAlerts[0].kind==AlertKind::SpeedCamera);
 assert(state.upcomingAlerts[1].kind==AlertKind::RedLightCamera&&state.upcomingAlerts[2].kind==AlertKind::SeatbeltCamera);
 puts("HLP camera 40 enforcement semantics, distinct camera types and four-lane masks passed");
}
