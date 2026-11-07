#include "audio/guidance_planner.h"
#include "audio/speech_message.h"
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace waze_hud;
HudState base(){HudState s;s.connected=s.hasProducerState=true;s.speedKmh=40;s.speedLimitKmh=50;return s;}
void discardLimit(GuidancePlanner &p,HudState &s,DeviceSettings &c){auto a=p.next(s,c,0);assert(a.kind==SpeechKind::CurrentLimit);p.complete(a,0);}
int main(){DeviceSettings c;
 {GuidancePlanner p;auto s=base();auto a=p.next(s,c,0);assert(a.kind==SpeechKind::CurrentLimit&&a.value==50);p.complete(a,0);
  for(int t=1;t<1000;++t)assert(p.next(s,c,t).kind==SpeechKind::None);
  s.speedLimitKmh=80;auto b=p.next(s,c,1001);assert(b.kind==SpeechKind::CurrentLimit&&b.key()!=a.key());assert(!p.canContinue(a,s,c,1001));}
 {GuidancePlanner p;auto s=base();discardLimit(p,s,c);s.navigationActive=true;s.maneuver=Maneuver::Right;s.maneuverDistanceM=350;
  std::strcpy(s.nextStreet.data(),"A");auto a=p.next(s,c,1);assert(a.kind==SpeechKind::Maneuver&&a.stage==1);
  auto words=speechMessage(a);assert(words.count==2&&words.prompts[0]==Prompt::Prepare&&words.prompts[1]==Prompt::Right);
  s.maneuverDistanceM=50;assert(!p.canContinue(a,s,c,2));a=p.next(s,c,2);assert(a.stage==2);p.complete(a,2);
  s.maneuverDistanceM=90;assert(p.next(s,c,3).kind==SpeechKind::None); // GPS wobble must not replay prepare.
  s.maneuverDistanceM=500;assert(p.next(s,c,4).kind==SpeechKind::Maneuver); // Next same-direction turn.
 }
 {GuidancePlanner p;auto s=base();discardLimit(p,s,c);s.nearestAlert={AlertKind::EnforcementCamera,700,0};assert(p.next(s,c,1).kind==SpeechKind::None);
  s.nearestAlert.distanceM=600;auto a=p.next(s,c,2);assert(a.kind==SpeechKind::Alert&&speechMessage(a).prompts[0]==Prompt::CameraEnforcement);
  s.nearestAlert={};assert(!p.canContinue(a,s,c,3)); // Removed camera cannot be queued or spoken later.
  s.nearestAlert={AlertKind::EnforcementCamera,590,0};a=p.next(s,c,4);p.complete(a,4);
  s.nearestAlert.distanceM=60;assert(p.next(s,c,5).kind==SpeechKind::None);
  s.nearestAlert.distanceM=500;assert(p.next(s,c,6).kind==SpeechKind::Alert); // Another camera of same kind.
 }
 {GuidancePlanner p;auto s=base();discardLimit(p,s,c);s.nearestAlert={AlertKind::SpeedDrop,900,80};auto a=p.next(s,c,1);
  assert(a.kind==SpeechKind::NextLimit&&a.value==80);auto m=speechMessage(a);assert(m.prompts[0]==Prompt::NextLimit&&m.prompts[1]==Prompt::N8&&m.prompts[2]==Prompt::Tens&&m.prompts[3]==Prompt::Kmh);
  p.complete(a,1);assert(p.next(s,c,2).kind==SpeechKind::None);
  a.kind=SpeechKind::CurrentLimit;a.value=105;m=speechMessage(a);assert(m.prompts[1]==Prompt::N1&&m.prompts[2]==Prompt::Hundred&&m.prompts[3]==Prompt::Odd&&m.prompts[4]==Prompt::N5);
  a.value=85;m=speechMessage(a);assert(m.prompts[3]==Prompt::FinalFive);
 }
 {GuidancePlanner p;auto s=base();s.speedKmh=71;discardLimit(p,s,c);assert(p.next(s,c,1499).kind==SpeechKind::None);
  auto a=p.next(s,c,1500);assert(a.kind==SpeechKind::Overspeed);p.complete(a,1600);
  assert(p.next(s,c,5599).kind==SpeechKind::None);assert(p.next(s,c,5600).kind==SpeechKind::Overspeed);
  s.speedKmh=60;s.speedLimitKmh=60;assert(!p.canContinue(a,s,c,5601));
 }
 {GuidancePlanner p;auto s=base();auto a=p.next(s,c,0);c.volume=0;assert(!p.canContinue(a,s,c,1));assert(p.next(s,c,2).kind==SpeechKind::None);
  c.volume=35;a=p.next(s,c,3);assert(a.kind==SpeechKind::CurrentLimit);
  s.signalStale=true;assert(!p.canContinue(a,s,c,4));assert(p.next(s,c,5).kind==SpeechKind::None);
  s.signalStale=false;s.sessionId=2;assert(p.next(s,c,6).kind==SpeechKind::CurrentLimit);
 }
 {GuidancePlanner p;auto s=base();auto a=p.next(s,c,0);s.nearestAlert={AlertKind::EnforcementCamera,100,0};
  assert(!p.canContinue(a,s,c,1));assert(p.next(s,c,1).kind==SpeechKind::Alert);}
 {GuidancePlanner p;auto s=base();auto a=p.next(s,c,0);s.sessionId=2;
  assert(!p.canContinue(a,s,c,1));assert(p.next(s,c,2).key()!=a.key());}
 {GuidancePlanner p;auto s=base();discardLimit(p,s,c);s.noPassingZone=true;auto a=p.next(s,c,1);assert(a.kind==SpeechKind::NoPassing);p.complete(a,1);assert(p.next(s,c,2).kind==SpeechKind::None);}
 puts("Guidance planner: turn stages/preemption, speed values, camera encounters, no-repeat, stale/session/mute cancellation and overspeed timing passed");
}
