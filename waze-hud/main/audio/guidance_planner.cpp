#include "audio/guidance_planner.h"
#include <algorithm>
#include <cstdlib>

namespace waze_hud {
namespace {
bool fresh(const HudState &s){return s.connected&&s.hasProducerState&&!s.signalStale;}
bool camera(AlertKind k){return k==AlertKind::SpeedCamera||k==AlertKind::RedLightCamera||
    (k>=AlertKind::EnforcementCamera&&k<=AlertKind::StopSignCamera);}
uint64_t turnHash(const HudState &s){
    uint64_t h=1469598103934665603ULL;
    auto add=[&](uint8_t b){h=(h^b)*1099511628211ULL;};
    add(static_cast<uint8_t>(s.maneuver));add(static_cast<uint8_t>(s.roundaboutExit));
    for(unsigned char b:s.nextStreet){if(!b)break;add(b);}return h;
}
}

void GuidancePlanner::observe(const HudState &s,const DeviceSettings &cfg,uint64_t now){
    auto reset=[&](){const auto sequence=serial_+1;*this=GuidancePlanner{repeatMs_};serial_=sequence;};
    if(!fresh(s)){reset();return;}
    if(!fresh_||session_!=s.sessionId){reset();fresh_=true;session_=s.sessionId;}
    if(s.speedLimitKmh!=limit_){limit_=s.speedLimitKmh;limitSerial_=++serial_;}
    if(s.navigationActive&&s.maneuver!=Maneuver::None&&s.maneuverDistanceM>=0){
        const uint64_t h=turnHash(s);
        if(h!=turnHash_||(turnDistance_>=0&&s.maneuverDistanceM>turnDistance_+180)){
            turnHash_=h;turnSerial_=++serial_;turnSpoken_=0;
        }
        turnDistance_=s.maneuverDistanceM;
    }else{turnHash_=0;turnDistance_=-1;turnSpoken_=0;}
    if(s.noPassingZone&&!zone_)zoneSerial_=++serial_;
    zone_=s.noPassingZone;
    const bool over=s.speedLimitKmh>0&&s.speedKmh>std::max(0,s.speedLimitKmh+int(cfg.overspeedOffsetKmh));
    if(over&&!over_){overSince_=now;overSpoken_=false;}
    over_=over;

    // For equal kind/value only track the nearest instance. A distance jump
    // after passing it starts a new encounter, even if the camera kind repeats.
    std::array<AlertState,kMaxAlerts+1> input{};input[0]=s.nearestAlert;
    for(auto &p:alerts_)p.present=false;
    const int count=1+std::min<std::size_t>(s.upcomingAlertCount,kMaxAlerts);
    for(int i=1;i<count;++i)input[i]=s.upcomingAlerts[i-1];
    for(int i=0;i<count;++i){const auto &a=input[i];
        if(a.kind==AlertKind::None||a.distanceM<0)continue;
        bool nearest=true;
        for(int j=0;j<count;++j)if(j!=i&&input[j].kind==a.kind&&input[j].valueKmh==a.valueKmh&&
            input[j].distanceM>=0&&(input[j].distanceM<a.distanceM||(input[j].distanceM==a.distanceM&&j<i)))nearest=false;
        if(!nearest)continue;
        AlertSlot *slot=nullptr;
        for(auto &p:alerts_)if(p.kind==a.kind&&p.value==a.valueKmh&&now-p.seen<=8000){slot=&p;break;}
        if(!slot){
            for(auto &p:alerts_)if(p.kind==AlertKind::None){slot=&p;break;}
            if(!slot)slot=&*std::min_element(alerts_.begin(),alerts_.end(),[](const AlertSlot &a,const AlertSlot &b){return a.seen<b.seen;});
            *slot={a.kind,a.valueKmh,a.distanceM,++serial_,now,false};
        }else{
            if(a.distanceM>slot->distance+180){slot->serial=++serial_;slot->spoken=false;}
            slot->distance=a.distanceM;slot->seen=now;
        }
        slot->present=true;
    }
}

int GuidancePlanner::candidates(const HudState &s,const DeviceSettings &cfg,uint64_t now,std::array<Announcement,12> &out)const{
    if(!fresh_||!fresh(s)||cfg.volume==0)return 0;
    int n=0;
    if(turnHash_&&turnDistance_<=500&&(s.maneuver!=Maneuver::Arrive||turnDistance_<=80)){
        const uint8_t stage=turnDistance_<=80||s.maneuver==Maneuver::Arrive?2:1;
        if(!(turnSpoken_&(1U<<stage)))out[n++]={SpeechKind::Maneuver,turnSerial_,stage,static_cast<uint8_t>(stage==2?120:95),0,s.roundaboutExit,s.maneuver};
    }
    for(const auto &a:alerts_)if(a.present&&!a.spoken){
        const bool speed=a.kind==AlertKind::SpeedDrop;
        const int range=speed?1000:camera(a.kind)?600:300;
        if(a.distance>range||(speed&&(a.value<=0||a.value>999))||(zone_&&a.kind==AlertKind::NoPassing))continue;
        out[n++]={speed?SpeechKind::NextLimit:SpeechKind::Alert,a.serial,0,
            static_cast<uint8_t>(a.distance<=150?105:camera(a.kind)?85:speed?55:75),a.value,0,Maneuver::None,a.kind};
    }
    if(zone_&&zoneSerial_!=zoneSpoken_)out[n++]={SpeechKind::NoPassing,zoneSerial_,0,80};
    if(over_&&now-overSince_>=1500&&(!overSpoken_||now-lastOverSpoken_>=repeatMs_))out[n++]={SpeechKind::Overspeed,0,0,60};
    if(limit_>0&&limit_<=999&&limitSerial_!=limitSpoken_)out[n++]={SpeechKind::CurrentLimit,limitSerial_,0,40,limit_};
    return n;
}

Announcement GuidancePlanner::next(const HudState &s,const DeviceSettings &cfg,uint64_t now){
    observe(s,cfg,now);std::array<Announcement,12> options{};const int n=candidates(s,cfg,now,options);
    if(!n)return {};
    return *std::max_element(options.begin(),options.begin()+n,[](const Announcement &a,const Announcement &b){return a.priority<b.priority;});
}
bool GuidancePlanner::canContinue(const Announcement &current,const HudState &s,const DeviceSettings &cfg,uint64_t now){
    observe(s,cfg,now);std::array<Announcement,12> options{};const int n=candidates(s,cfg,now,options);
    bool found=false;
    for(int i=0;i<n;++i){
        if(options[i].key()==current.key())found=true;
        if(options[i].priority>=90&&options[i].priority>current.priority)return false;
    }
    return found;
}
void GuidancePlanner::complete(const Announcement &a,uint64_t now){
    switch(a.kind){
        case SpeechKind::Maneuver:if(a.source==turnSerial_)turnSpoken_|=a.stage==2?6:2;break;
        case SpeechKind::CurrentLimit:limitSpoken_=a.source;break;
        case SpeechKind::NextLimit:case SpeechKind::Alert:{
            for(auto &p:alerts_)if(p.serial==a.source)p.spoken=true;
            break;}
        case SpeechKind::NoPassing:zoneSpoken_=a.source;break;
        case SpeechKind::Overspeed:lastOverSpoken_=now;overSpoken_=true;break;
        default:break;
    }
}
}
