#pragma once

namespace waze_hud {
struct ScreenPoint { int x; int y; };
// ES3C28P keeps the LCD in its native 240x320 address space.
inline ScreenPoint toNative(ScreenPoint p, bool portrait, bool rotated, bool mirrored) {
    const int w = portrait ? 240 : 320, h = portrait ? 320 : 240;
    if (rotated) { p.x = w - 1 - p.x; p.y = h - 1 - p.y; }
    if (mirrored) p.x = w - 1 - p.x;
    return portrait ? p : ScreenPoint{p.y, p.x};
}
inline ScreenPoint fromNative(ScreenPoint p, bool portrait, bool rotated, bool mirrored) {
    if (!portrait) p = {p.y, p.x};
    const int w = portrait ? 240 : 320, h = portrait ? 320 : 240;
    if (mirrored) p.x = w - 1 - p.x;
    if (rotated) { p.x = w - 1 - p.x; p.y = h - 1 - p.y; }
    return p;
}
}
