#include "audio/speech_bank.h"
#include "audio/speech_message.h"
#include "audio/mulaw.h"
#include "audio/pcm_gain.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
using namespace waze_hud;
void word(FILE *f,uint32_t v,int bytes){for(int i=0;i<bytes;++i)std::fputc((v>>(8*i))&255,f);}
void wav(const std::string &path,const SpeechMessage &message){
    std::vector<int16_t> pcm;
    for(std::size_t w=0;w<message.count;++w){const auto clip=speechClip(message.prompts[w]);
        for(std::size_t i=0;i<clip.samples;++i){const int16_t sample=scalePcm(int32_t(decodeMulaw(clip.mulaw[i]))*85/100,35);pcm.push_back(sample);pcm.push_back(sample);}
        pcm.insert(pcm.end(),400,0);
    }
    FILE *f=std::fopen(path.c_str(),"wb");assert(f);
    std::fwrite("RIFF",1,4,f);word(f,36+pcm.size()*2,4);std::fwrite("WAVEfmt ",1,8,f);word(f,16,4);word(f,1,2);word(f,1,2);
    word(f,16000,4);word(f,32000,4);word(f,2,2);word(f,16,2);std::fwrite("data",1,4,f);word(f,pcm.size()*2,4);
    for(int16_t sample:pcm)word(f,uint16_t(sample),2);std::fclose(f);
}
int main(int argc,char **argv){
    assert(argc==3);FILE *reference=std::fopen(argv[1],"rb");assert(reference);
    for(int c=0;c<256;++c){int lo=std::fgetc(reference),hi=std::fgetc(reference);assert(lo>=0&&hi>=0);assert(decodeMulaw(c)==static_cast<int16_t>(lo|(hi<<8)));}
    assert(std::fgetc(reference)==EOF);std::fclose(reference);
    std::size_t total=0;
    for(int p=0;p<int(Prompt::Count);++p){const auto clip=speechClip(static_cast<Prompt>(p));assert(clip.mulaw&&clip.samples>100&&clip.samples<80000);
        int peak=0;for(std::size_t i=0;i<clip.samples;++i)peak=std::max(peak,std::abs(int(decodeMulaw(clip.mulaw[i]))));assert(peak>500);total+=clip.samples;
    }
    const std::string out=argv[2];
    Announcement a;a.kind=SpeechKind::Maneuver;a.maneuver=Maneuver::Right;a.stage=1;wav(out+"/prepare-right.wav",speechMessage(a));
    a.kind=SpeechKind::CurrentLimit;a.value=80;wav(out+"/current-limit-80.wav",speechMessage(a));
    a.kind=SpeechKind::NextLimit;a.value=50;wav(out+"/next-limit-50.wav",speechMessage(a));
    a.kind=SpeechKind::Alert;a.alert=AlertKind::EnforcementCamera;wav(out+"/camera-enforcement.wav",speechMessage(a));
    std::printf("All 256 mu-law codes match FFmpeg, %d real Vietnamese clips (%zu bytes) contain audio; four actual-asset speech samples exported at 35%%\n",int(Prompt::Count),total);
}
