// Host harness for the ES3C28P renderer (320x240): renders HUD scenarios through the real HudRenderer.
#include "display/display_driver.h"
#include "display/hud_renderer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <cassert>

static int64_t g_nowUs = 1000000;
int64_t esp_timer_get_time() { return g_nowUs; }
void *heap_caps_malloc(size_t n, unsigned) { return std::malloc(n); }
void heap_caps_free(void *p) { std::free(p); }
const char *esp_err_to_name(esp_err_t) { return "err"; }
#ifdef _WIN32
extern "C" struct tm *gmtime_r(const time_t *t, struct tm *r) { gmtime_s(r, t); return r; }
#endif

static std::vector<uint8_t> g_clockOptimus;
static std::vector<uint8_t> g_clockBumblebee;
const uint8_t *clockOptimusStart = nullptr;
const uint8_t *clockBumblebeeStart = nullptr;
static uint32_t g_randomValue = 0;
uint32_t esp_random() { return g_randomValue; }

namespace waze_hud {
static std::deque<TouchEvent> g_events;
static DeviceSettings g_config;
DeviceConfig &DeviceConfig::instance() { static DeviceConfig c; return c; }
DeviceSettings DeviceConfig::snapshot() const { return g_config; }
esp_err_t DeviceConfig::setVolume(uint8_t v, bool) { g_config.volume=v; ++g_config.revision; return ESP_OK; }
esp_err_t DeviceConfig::persistVolume() { return ESP_OK; }
TouchSensor &TouchSensor::instance() { static TouchSensor t; return t; }
TouchOtaConfirmStatus TouchSensor::confirmStatus() const { return {}; }
TouchEvent TouchSensor::takeEvent() { if(g_events.empty())return TouchEvent::None; auto e=g_events.front();g_events.pop_front();return e; }
static int g_width=320,g_height=240;
static uint16_t g_fb[320 * 240];
DisplayDriver &DisplayDriver::instance() { static DisplayDriver d; return d; }
esp_err_t DisplayDriver::drawRegion(const Rect &r, uint16_t *px) {
    assert(r.x>=0 && r.y>=0 && r.x+r.width<=g_width && r.y+r.height<=g_height);
    for (int y = 0; y < r.height; ++y)
        for (int x = 0; x < r.width; ++x) g_fb[(r.y + y) * g_width + r.x + x] = px[y * r.width + x];
    return 0;
}
esp_err_t DisplayDriver::setBrightness(uint8_t) { return 0; }
esp_err_t DisplayDriver::setOrientation(bool, bool, bool portrait) { g_width=portrait?240:320;g_height=portrait?320:240;return 0; }
esp_err_t DisplayDriver::setInvertColor(bool) { return 0; }
template <std::size_t N> void setText(std::array<char, N> &t, const char *s) { std::snprintf(t.data(), t.size(), "%s", s); }

static HudState base() {
    HudState s{};
    s.connected = true; s.hasProducerState = true; s.navigationActive = true;
    s.speedKmh = 40; s.speedLimitKmh = 50; s.maneuver = Maneuver::Left; s.maneuverDistanceM = 350;
    setText(s.currentStreet, "Đường Lê Lợi"); setText(s.nextStreet, "Võ Văn Kiệt"); setText(s.eta, "20:01");
    s.remainingMinutes = 25; s.remainingMeters = 12400; s.remainingKm = 12.4f;
    s.clockUnixSeconds = 1787478627; s.timezoneOffsetMinutes = 420;
    return s;
}
static void lanes(HudState &s) { s.laneCount = 3; s.lanes[0] = {0x05, 0x04}; s.lanes[1] = {0x01, 0x01}; s.lanes[2] = {0x30, 0x10}; }
static void alerts(HudState &s, std::initializer_list<AlertState> list) {
    int i = 0;
    for (const AlertState &a : list) {
        if (i == 0) s.nearestAlert = a; else if (i - 1 < (int)kMaxAlerts) s.upcomingAlerts[i - 1] = a;
        ++i;
    }
    s.upcomingAlertCount = i > 1 ? (uint8_t)(i - 1) : 0;
}
static void dump(const char *name) {
    std::string path = std::string(getenv("OUTDIR")) + "/" + name + ".ppm";
    FILE *f = std::fopen(path.c_str(), "wb");
    std::fprintf(f, "P6\n%d %d\n255\n",g_width,g_height);
    for (int i = 0; i < 320 * 240; ++i) {
        uint16_t p = g_fb[i];
        unsigned char rgb[3] = {(unsigned char)(((p >> 11) & 31) * 255 / 31), (unsigned char)(((p >> 5) & 63) * 255 / 63), (unsigned char)((p & 31) * 255 / 31)};
        std::fwrite(rgb, 1, 3, f);
    }
    std::fclose(f);
}
static void shot(const char *name, const HudState &st, DeviceSettings set = {}, SystemStatusSnapshot sys = {}) {
    std::memset(g_fb, 0x55, sizeof(g_fb));
    g_config=set;
    HudRenderer r; r.init();
    r.render(st, set, sys);
    dump(name);
}
}  // namespace waze_hud

int main() {
    using namespace waze_hud;
    const char *optimusPath = std::getenv("CLOCK_OPTIMUS");
    const char *bumblebeePath = std::getenv("CLOCK_BUMBLEBEE");
    if (!optimusPath || !bumblebeePath) return 2;
    std::ifstream optimusAsset(optimusPath, std::ios::binary);
    std::ifstream bumblebeeAsset(bumblebeePath, std::ios::binary);
    g_clockOptimus.assign(std::istreambuf_iterator<char>(optimusAsset), {});
    g_clockBumblebee.assign(std::istreambuf_iterator<char>(bumblebeeAsset), {});
    if (g_clockOptimus.size() != 320U * 240U * sizeof(uint16_t) ||
        g_clockBumblebee.size() != 320U * 240U * sizeof(uint16_t)) return 3;
    clockOptimusStart = g_clockOptimus.data();
    clockBumblebeeStart = g_clockBumblebee.data();
    g_nowUs = 1000000;  // (1000 ms / 250) % 2 == 0: the overspeed blink is in its "on" phase
    { auto s = base(); shot("n00_nav_no_alert", s); }
    { auto s = base(); alerts(s, {{AlertKind::SpeedCamera, 300, 0}}); shot("n01_one_alert", s); }
    { auto s = base(); alerts(s, {{AlertKind::SpeedCamera, 300, 0}, {AlertKind::Police, 850, 0}}); shot("n02_two_alerts", s); }
    { auto s = base(); alerts(s, {{AlertKind::SpeedCamera, 300, 0}, {AlertKind::Police, 850, 0}, {AlertKind::Hazard, 1400, 0}, {AlertKind::Railway, 1800, 0}, {AlertKind::Flood, 2600, 0}}); shot("n03_one_card", s); }
    { auto s = base(); alerts(s, {{AlertKind::SpeedCamera, 1200, 0}}); shot("n04_far_only", s); }
    g_nowUs = 295000000;  // 295 s: inside the 10 s trip peek that ends every 5 minutes
    { auto s = base(); shot("p00_peek_plain", s); }
    { auto s = base(); alerts(s, {{AlertKind::SpeedCamera, 300, 0}, {AlertKind::Police, 850, 0}, {AlertKind::Hazard, 1400, 0}}); shot("p01_peek_alert_card", s); }
    { auto s = base(); lanes(s); shot("p02_peek_lanes", s); }
    g_nowUs = 1000000;
    { auto s = base(); s.navigationActive = false; s.maneuver = Maneuver::None; s.nextStreet.fill(0); s.eta.fill(0); s.remainingMinutes = 0; s.remainingKm = 0; s.remainingMeters = 0; alerts(s, {{AlertKind::Police, 1100, 0}}); shot("n04b_free_one_card", s); }
    { auto s = base(); lanes(s); alerts(s, {{AlertKind::SpeedCamera, 300, 0}, {AlertKind::Police, 850, 0}}); shot("n05_lanes_alerts", s); }
    { auto s = base(); s.laneCount = 5; for (uint8_t i = 0; i < 5; ++i) { s.lanes[i].directionMask = i == 0 ? 0x05 : i == 4 ? 0x30 : 0x01; s.lanes[i].selectedMask = i == 2 ? 0x01 : 0; } shot("n05c_five_lanes", s); }
    { auto s = base(); s.laneCount = 8; for (uint8_t i = 0; i < 8; ++i) { s.lanes[i].directionMask = i < 2 ? 0x04 : i > 5 ? 0x20 : 0x01; s.lanes[i].selectedMask = i == 3 || i == 4 ? 0x01 : 0; } alerts(s, {{AlertKind::SpeedCamera, 300, 0}, {AlertKind::Police, 850, 0}, {AlertKind::Hazard, 1400, 0}}); shot("n05d_eight_lanes_alerts", s); }
    { auto s = base(); lanes(s); shot("n05e_three_lanes_plain", s); }
    { auto s = base(); s.laneCount = 6; for (uint8_t i = 0; i < 6; ++i) { s.lanes[i].directionMask = i < 2 ? 0x04 : i > 3 ? 0x20 : 0x01; s.lanes[i].selectedMask = i == 2 || i == 3 ? 0x01 : 0; } shot("n05b_six_lanes", s); }
    { auto s = base(); alerts(s, {{AlertKind::TrafficJam, 420, 0, 4, 18}, {AlertKind::TrafficJam, 1350, 0, 2, 5}}); shot("n06_traffic", s); }
    { auto s = base(); s.noPassingZone = true; s.noPassingRemainingM = 1250; alerts(s, {{AlertKind::SpeedCamera, 600, 0}}); shot("n07_nopassing", s); }
    { auto s = base(); s.speedKmh = 86; s.speedLimitKmh = 60; s.overSpeed = true; s.maneuver = Maneuver::SharpLeft; s.maneuverDistanceM = 35; shot("n08_overspeed", s); }
    { auto s = base(); s.maneuver = Maneuver::RoundaboutRight; s.roundaboutExit = 3; s.maneuverDistanceM = 1200; shot("n09_roundabout", s); }
    { auto s = base(); setText(s.currentStreet, "Đường Cách Mạng Tháng Tám, Phường Bến Thành"); setText(s.nextStreet, "Đại lộ Võ Văn Kiệt, Phường Cầu Ông Lãnh"); shot("n10_long_names", s); }
    { auto s = base(); s.speedKmh = 130; s.speedLimitKmh = 120; shot("n11_three_digit", s); }
    { auto s = base(); s.navigationActive = false; s.maneuver = Maneuver::None; s.nextStreet.fill(0); s.eta.fill(0); shot("f00_free_drive", s); }
    { auto s = base(); s.navigationActive = false; s.maneuver = Maneuver::None; s.nextStreet.fill(0); s.eta.fill(0); alerts(s, {{AlertKind::Police, 400, 0}, {AlertKind::SpeedCamera, 1100, 0}}); shot("f01_free_drive_alerts", s); }
    g_randomValue = 0;
    { HudState s{}; shot("s00_waiting_optimus", s); }
    { auto s = base(); s.connected = false; s.hasProducerState = false; shot("s00_clock_optimus", s); }
    g_randomValue = 1;
    { HudState s{}; shot("s00_waiting_bumblebee", s); }
    { auto s = base(); s.connected = false; s.hasProducerState = false; shot("s00_clock_bumblebee", s); }
    { HudState s{}; s.connected = true; shot("s01_connected_waiting", s); }
    { SystemStatusSnapshot y{}; y.visible = true; y.bleConnected = true; y.bleRssiDbm = -62; shot("s02_system_status", base(), {}, y); }
    { DeviceSettings d{}; d.theme = UiTheme::Night; shot("s03_night", base(), d); }
    DeviceSettings portrait{}; portrait.portraitDisplay=true;
    { auto s=base(); shot("v01_navigation",s,portrait); lanes(s);shot("v02_lanes",s,portrait);
      alerts(s,{{AlertKind::SpeedCamera,300,0},{AlertKind::Police,600,0}});shot("v03_lanes_alert",s,portrait);
      s.maneuver=Maneuver::RoundaboutRight;s.roundaboutExit=4;shot("v04_roundabout",s,portrait);
      s.speedKmh=130;s.speedLimitKmh=120;shot("v05_overspeed",s,portrait);
      s.signalStale=true;s.hasProducerState=false;shot("v06_stale",s,portrait); }
    {auto s=base();s.navigationActive=false;s.maneuver=Maneuver::None;shot("v07_free",s,portrait);
      alerts(s,{{AlertKind::Police,300,0},{AlertKind::TrafficJam,600,0,4,18},{AlertKind::Hazard,900,0}});
      shot("v08_free_alerts",s,portrait);}
    {auto s=base();s.connected=false;s.hasProducerState=false;g_randomValue=0;shot("v09_clock_optimus",s,portrait);
      g_randomValue=1;shot("v10_clock_bumblebee",s,portrait);s.clockUnixSeconds=0;shot("v11_cold_boot",s,portrait);}
    {SystemStatusSnapshot sys{};sys.visible=true;sys.bleConnected=true;sys.bleRssiDbm=-62;shot("v12_status",base(),portrait,sys);}
    {auto s=base();s.laneCount=16;for(int i=0;i<16;i++)s.lanes[i]={0x35,static_cast<uint8_t>(i==9?1:0)};
      setText(s.nextStreet,"Đại lộ Võ Văn Kiệt, Phường Cầu Ông Lãnh");shot("v13_many_lanes_long_street",s,portrait);}
    {auto s=base();s.maneuver=Maneuver::None;shot("v14_no_maneuver",s,portrait);}
    {auto s=base(); HudRenderer r;g_config=portrait;r.init();
      for(int m=0;m<=20;++m){s.maneuver=static_cast<Maneuver>(m);s.localGeneration++;g_nowUs+=100000;r.render(s,g_config,{});}
      for(int a=1;a<=75;++a){s.nearestAlert={static_cast<AlertKind>(a),150,50,4,18};s.localGeneration++;g_nowUs+=100000;r.render(s,g_config,{});}
      std::puts("All 21 maneuver codes and 75 alert codes rendered in portrait");}
    {auto s=base();s.speedKmh=50;s.maneuver=Maneuver::Roundabout;s.roundaboutExit=4;s.maneuverDistanceM=0;
      alerts(s,{{AlertKind::SpeedDrop,1500,80},{AlertKind::Police,1800,0},{AlertKind::Hazard,2200,0},{AlertKind::Railway,2500,0}});
      shot("v20_device_regression",s,portrait);lanes(s);shot("v21_multiple_alerts_lanes",s,portrait);}
    {auto s=base();s.maneuverDistanceM=0;g_config=portrait;HudRenderer r;r.init();r.render(s,g_config,{});
      g_events.push_back(TouchEvent::Tap);g_nowUs+=100000;r.render(s,g_config,{});dump("v15_tap_clock");
      const auto clockPixel=g_fb[200*240+50];
      g_nowUs+=6000000;r.render(s,g_config,{});dump("v16_existing_turn_stays_clock");
      assert(g_fb[200*240+50]==clockPixel); // Existing 0m turn must not cancel manual clock.
      s.maneuver=Maneuver::Right;s.localGeneration++;g_nowUs+=100000;r.render(s,g_config,{});dump("v22_new_turn_returns_hud");
      assert(g_fb[200*240+50]!=clockPixel);
      g_events.push_back(TouchEvent::Hold);s.maneuverDistanceM=600;g_nowUs+=100000;r.render(s,g_config,{});dump("v17_trip");
      g_events.push_back(TouchEvent::Tap);g_config.volume=5;g_events.push_back(TouchEvent::VolumeDown);
      g_nowUs+=100000;r.render(s,g_config,{});g_nowUs+=100000;r.render(s,g_config,{});dump("v18_mute");
      assert(g_config.volume==0);g_events.push_back(TouchEvent::Release);r.render(s,g_config,{});
      g_config.portraitDisplay=false;g_nowUs+=2000000;r.render(s,g_config,{});dump("v19_switch_landscape");}
    std::puts("done");
}
