#include "audio/pcm_gain.h"
#include "display/auto_return.h"
#include <cassert>
#include <cstdio>
using namespace waze_hud;
int main(){
    for(unsigned volume:{0U,5U,10U,25U,50U,75U,100U})for(int sample=-32768;sample<=32767;++sample){
        const int expected=sample*static_cast<int>(volume)/100;
        assert(scalePcm(static_cast<int16_t>(sample),volume)==expected);
        if(sample<0)assert(scalePcm(sample,volume)<=0);
    }
    assert(scalePcm(-20000,50)==-10000);assert(scalePcm(-1000,50)==-500);
    assert(scalePcm(-32768,101)==-32768);
    AutoReturn policy;
    assert(policy.update(0,42));
    policy.manualView(100,42);
    assert(!policy.update(100,42));assert(!policy.update(10000,42));
    assert(policy.update(10100,84));assert(!policy.update(10200,84));
    policy.manualView(11000,84);
    assert(!policy.update(12000,126));assert(!policy.update(15999,126));
    assert(policy.update(16000,126));assert(!policy.update(16100,126));
    policy.manualView(17000,126);assert(!policy.update(18000,0));
    assert(policy.update(23000,126));
    puts("458752 signed PCM checks and manual-view/new-event return regression tests passed");
}
