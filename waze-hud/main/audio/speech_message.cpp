#include "audio/speech_message.h"
namespace waze_hud {
namespace {
Prompt digit(int n){return static_cast<Prompt>(static_cast<int>(Prompt::N0)+n);}
void number(SpeechMessage &m,int n){
    if(n<0||n>999)return;
    const int hundreds=n/100,tens=(n%100)/10,ones=n%10;
    if(hundreds){m.add(digit(hundreds));m.add(Prompt::Hundred);}
    if(tens>=2){m.add(digit(tens));m.add(Prompt::Tens);}
    else if(tens==1)m.add(Prompt::Ten);
    else if(hundreds&&ones)m.add(Prompt::Odd);
    if(ones||n==0)m.add(tens>=2&&ones==1?Prompt::FinalOne:tens>=1&&ones==5?Prompt::FinalFive:digit(ones));
}
Prompt turn(Maneuver m){
    switch(m){
        case Maneuver::Left:return Prompt::Left;
        case Maneuver::Right:return Prompt::Right;
        case Maneuver::SlightLeft:return Prompt::SlightLeft;
        case Maneuver::SlightRight:return Prompt::SlightRight;
        case Maneuver::SharpLeft:return Prompt::SharpLeft;
        case Maneuver::SharpRight:return Prompt::SharpRight;
        case Maneuver::UTurn:case Maneuver::UTurnRightReserved:return Prompt::UTurn;
        case Maneuver::KeepLeft:return Prompt::KeepLeft;
        case Maneuver::KeepRight:return Prompt::KeepRight;
        case Maneuver::ExitLeft:return Prompt::ExitLeft;
        case Maneuver::ExitRight:return Prompt::ExitRight;
        case Maneuver::Arrive:return Prompt::Arrive;
        case Maneuver::FerryReserved:return Prompt::Ferry;
        case Maneuver::Roundabout:case Maneuver::RoundaboutLeft:case Maneuver::RoundaboutRight:
        case Maneuver::RoundaboutStraight:case Maneuver::RoundaboutUTurn:return Prompt::Roundabout;
        default:return Prompt::Straight;
    }
}
Prompt alert(AlertKind a){
    switch(a){
        case AlertKind::SpeedCamera:return Prompt::CameraSpeed;
        case AlertKind::EnforcementCamera:return Prompt::CameraEnforcement;
        case AlertKind::RedLightCamera:return Prompt::CameraRedLight;
        case AlertKind::DummyCamera:case AlertKind::SeatbeltCamera:case AlertKind::DistanceCamera:
        case AlertKind::BusLaneCamera:case AlertKind::NoiseCamera:case AlertKind::StopSignCamera:return Prompt::CameraOther;
        case AlertKind::Police:return Prompt::Police;
        case AlertKind::Hazard:return Prompt::Hazard;
        case AlertKind::Accident:return Prompt::Accident;
        case AlertKind::RoadClosed:return Prompt::RoadClosed;
        case AlertKind::Railway:return Prompt::Railway;
        case AlertKind::Construction:return Prompt::Construction;
        case AlertKind::NoPassing:return Prompt::GenericAlert; // Ahead, not necessarily inside the zone yet.
        case AlertKind::SpeedBump:return Prompt::SpeedBump;
        case AlertKind::SchoolZone:return Prompt::SchoolZone;
        case AlertKind::TrafficLight:return Prompt::TrafficLight;
        case AlertKind::TrafficJam:return Prompt::TrafficJam;
        default:return Prompt::GenericAlert;
    }
}
}
SpeechMessage speechMessage(const Announcement &a){
    SpeechMessage m;
    switch(a.kind){
        case SpeechKind::Maneuver:
            if(a.stage==1)m.add(Prompt::Prepare);
            m.add(turn(a.maneuver));
            if(a.exit>0&&a.exit<=999&&(a.maneuver==Maneuver::Roundabout||a.maneuver==Maneuver::RoundaboutLeft||
                a.maneuver==Maneuver::RoundaboutRight||a.maneuver==Maneuver::RoundaboutStraight||a.maneuver==Maneuver::RoundaboutUTurn)){
                m.add(Prompt::RoundaboutExit);number(m,a.exit);
            }break;
        case SpeechKind::CurrentLimit:case SpeechKind::NextLimit:
            m.add(a.kind==SpeechKind::CurrentLimit?Prompt::CurrentLimit:Prompt::NextLimit);number(m,a.value);m.add(Prompt::Kmh);break;
        case SpeechKind::Alert:m.add(alert(a.alert));break;
        case SpeechKind::NoPassing:m.add(Prompt::NoPassing);break;
        default:break;
    }
    return m;
}
}
