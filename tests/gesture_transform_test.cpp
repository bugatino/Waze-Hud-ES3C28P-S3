#include "touch/gesture.h"
#include "display/screen_transform.h"
#include <cassert>
#include <cstdio>
using namespace waze_hud;
int main() {
    assert(toNative({0,0},true,false,false).x==239);
    assert(toNative({0,0},true,false,true).x==0);
    assert(fromNative({239,0},true,false,false).x==0);
    assert(toNative({0,0},false,false,false).x==0);
    for(bool portrait:{false,true})for(bool rotate:{false,true})for(bool mirror:{false,true}){
        const int w=portrait?240:320,h=portrait?320:240;
        for(int y=0;y<h;++y)for(int x=0;x<w;++x){
            const auto n=toNative({x,y},portrait,rotate,mirror);
            assert(n.x>=0&&n.x<240&&n.y>=0&&n.y<320);
            const auto p=fromNative(n,portrait,rotate,mirror);assert(p.x==x&&p.y==y);
        }
        auto from=toNative({80,150},portrait,rotate,mirror);
        auto to=toNative({80,100},portrait,rotate,mirror);
        assert(fromNative(to,portrait,rotate,mirror).y < fromNative(from,portrait,rotate,mirror).y);
    }
    Gesture g;
    assert(g.update(true,10,10,0)==TouchEvent::None);
    assert(g.update(false,10,10,100)==TouchEvent::Tap);
    g.update(true,50,100,200);
    assert(g.update(true,50,79,240)==TouchEvent::VolumeUp);
    assert(g.update(true,50,58,280)==TouchEvent::VolumeUp);
    assert(g.update(false,50,58,320)==TouchEvent::Release);
    g.update(true,50,100,400);
    assert(g.update(true,50,122,450)==TouchEvent::VolumeDown);
    assert(g.update(false,50,122,500)==TouchEvent::Release);
    g.update(true,50,100,600);
    assert(g.update(true,50,100,1400)==TouchEvent::Hold);
    assert(g.update(false,50,100,1500)==TouchEvent::None);
    g.update(true,50,100,1600);g.update(true,90,100,1700);
    assert(g.update(false,90,100,1800)==TouchEvent::None);
    g.update(true,10,10,2000);g.cancel();
    assert(g.update(false,10,10,2100)==TouchEvent::None);
    std::puts("Gesture and all 614400 orientation mappings passed");
}
