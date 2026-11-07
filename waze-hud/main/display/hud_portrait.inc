// Included by hud_renderer.cpp: shares the existing maneuver/alert/lane assets.
namespace {
void portraitCard(Canvas &c, int x, int y, int w, int h, uint16_t border) {
    c.fillRect(x,y,w,h,colors::rgb565(16,32,48));
    drawCard(c,x,y,w,h,border==colors::Muted?colors::rgb565(45,67,85):border);
}
void wrappedStreet(Canvas &canvas, int x, int y, const char *value, int width) {
    if (canvas.fontTextWidth(value, assets::kTextSmall) <= width) {
        canvas.fontText(x, y, value, assets::kTextSmall, colors::White, width, false);
        return;
    }
    char first[164]{};
    int split = 0;
    for (int i = 0; value[i] && i < 160; ++i) {
        first[i] = value[i]; first[i + 1] = 0;
        if (value[i] == ' ' && canvas.fontTextWidth(first, assets::kTextSmall) <= width) split = i;
        if (canvas.fontTextWidth(first, assets::kTextSmall) > width) break;
    }
    if (split > 0) {
        first[split] = 0;
        canvas.fontText(x, y, first, assets::kTextSmall, colors::White, width, false);
        canvas.fontText(x, y + 16, value + split + 1, assets::kTextSmall, colors::White, width, false);
    } else canvas.fontText(x, y, value, assets::kTextSmall, colors::White, width, false);
}
void angularPanel(Canvas &c, int x, int y, int w, int h, uint16_t color) {
    c.fillRect(x + 7, y, w - 14, h, colors::Panel);
    c.fillRect(x, y + 7, w, h - 14, colors::Panel);
    const int xs[] = {x+7,x+w-8,x+w-1,x+w-1,x+w-8,x+7,x,x,x+7};
    const int ys[] = {y,y,y+7,y+h-8,y+h-1,y+h-1,y+h-8,y+7,y};
    for (int i=0;i<8;++i) c.line(xs[i],ys[i],xs[i+1],ys[i+1],color);
}
}

void HudRenderer::renderVolume(Canvas &c, const Rect &r, int width, int height,
                               const DeviceSettings &settings) {
    const int x = (width - 204) / 2 - r.x, y = (height - 106) / 2 - r.y;
    c.fillRect(x, y, 204, 106, colors::Background);
    portraitCard(c,x,y,204,106,settings.volume ? colors::Cyan : colors::Amber);
    c.fontText(x,y+7, settings.volume ? "ÂM LƯỢNG LOA" : "ĐÃ TẮT TIẾNG",
               assets::kTextSmall,colors::Cyan,204,true);
    char volume[12]; std::snprintf(volume,sizeof(volume),"%u%%",settings.volume);
    c.fontText(x,y+28,volume,assets::kTextLarge,colors::White,204,true);
    c.fillRect(x+22,y+66,160,6,colors::Muted);
    c.fillRect(x+22,y+66,160*settings.volume/100,6,colors::Cyan);
    c.fontText(x,y+79,"Vuốt lên / xuống để chỉnh",assets::kTextSmall,colors::Muted,204,true);
}

void HudRenderer::renderTripPage(Canvas &c, const Rect &r, int width, int height) {
    c.clear(colors::rgb565(7,14,23));
    const auto trip = trip_.display();
    c.fontText(-r.x,12-r.y,"THỐNG KÊ CHUYẾN ĐI",assets::kTextMedium,colors::Cyan,width,true);
    const int top = height == 320 ? 86 : 50;
    const int cellW = (width-30)/2, cellH = height == 320 ? 78 : 70;
    char distance[24], duration[24], maxSpeed[24], avg[24];
    std::snprintf(distance,sizeof(distance),"%d.%d km",trip.distanceTenthsKm/10,trip.distanceTenthsKm%10);
    std::snprintf(duration,sizeof(duration),"%d phút",trip.minutes);
    std::snprintf(maxSpeed,sizeof(maxSpeed),"%d km/h",trip.maxKmh);
    std::snprintf(avg,sizeof(avg),"%d km/h",trip.avgKmh);
    const char *labels[]={"ĐÃ ĐI","THỜI GIAN","CAO NHẤT","TRUNG BÌNH"};
    const char *values[]={distance,duration,maxSpeed,avg};
    for(int i=0;i<4;++i){
        const int x=10+(i%2)*(cellW+10)-r.x, y=top+(i/2)*(cellH+10)-r.y;
        portraitCard(c,x,y,cellW,cellH,colors::Muted);
        c.fontText(x,y+7,labels[i],assets::kTextSmall,colors::Muted,cellW,true);
        c.fontText(x,y+31,values[i],assets::kTextMedium,i==0?colors::Cyan:colors::White,cellW,true);
    }
    c.fontText(-r.x,height-24-r.y,"Chạm: trở về HUD",assets::kTextSmall,colors::Muted,width,true);
}

void HudRenderer::renderPortrait(Canvas &c, const Rect &r, const HudState &s,
                                 const DeviceSettings &settings, const SystemStatusSnapshot &sys) {
    c.clear(colors::rgb565(7,14,23));
    auto txt = [&](int x,int y,const char *v,const assets::BitmapFont &f,uint16_t col,int w=-1,bool center=false){
        c.fontText(x,y,v,f,col,w,center);
    };
    const bool fresh = s.connected && s.hasProducerState && !s.signalStale;
    const uint16_t accent = bumblebeeClock_ ? colors::Amber : colors::Cyan;
    const int64_t ms = localClockMillis(s);
    tm local{};
    char time[8]="--:--", date[48]="Chưa đồng bộ giờ";
    if(ms!=INT64_MIN){
        const time_t sec=ms/1000; gmtime_r(&sec,&local);
        std::snprintf(time,sizeof(time),"%02d:%02d",local.tm_hour,local.tm_min);
        lunar_date_t lunar{};
        const char *days[]={"CN","T2","T3","T4","T5","T6","T7"};
        if(lunar_from_unix(currentClockUnixSeconds(s),&lunar))
            std::snprintf(date,sizeof(date),"%s - %02d/%02d - ÂL %02d/%02d%s",days[local.tm_wday],local.tm_mday,
                local.tm_mon+1,lunar.day,lunar.month,lunar.leap_month?"n":"");
    }
    if(sys.visible){
        txt(0,12,"TRẠNG THÁI THIẾT BỊ",assets::kTextMedium,colors::Cyan,240,true);
        char detail[80];
        txt(15,58,"WAZE HUD - ESP32-S3",assets::kTextMedium,colors::White,210);
        std::snprintf(detail,sizeof(detail),"BLE: %s",sys.bleConnected?"Đã kết nối":"Chưa kết nối");
        txt(15,100,detail,assets::kTextMedium,sys.bleConnected?colors::Green:colors::Amber,210);
        std::snprintf(detail,sizeof(detail),"Tín hiệu: %d dBm",sys.bleRssiDbm);
        if(sys.bleConnected)txt(15,134,detail,assets::kTextMedium,colors::White,210);
        txt(15,168,"Hiển thị: Dọc 240 x 320",assets::kTextSmall,colors::White,210);
        std::snprintf(detail,sizeof(detail),"Loa: %u%%",settings.volume);
        txt(15,203,detail,assets::kTextMedium,colors::White,210);
        txt(15,240,ms==INT64_MIN?"Giờ: Chưa đồng bộ":"Giờ: Đã đồng bộ",assets::kTextSmall,colors::Muted,210);
        return;
    }
    if(page_==2){renderTripPage(c,{0,0,240,320},240,320);}
    else if(page_==1 || ((!s.connected || !s.hasProducerState) && !s.signalStale)){
        const auto *bg=reinterpret_cast<const uint16_t *>(bumblebeeClock_?clockBumblebeeStart:clockOptimusStart);
        // Use the approved robot artwork's upper section, without duplicating
        // its old landscape clock frame. No extra framebuffer is required.
        for(int y=std::max<int>(0,r.y-settings.offsetY);y<std::min<int>(164,r.y+r.height-settings.offsetY);++y)
            for(int x=0;x<240;++x) c.pixel(x,y,bg[y*320+(x*320/240)]);
        angularPanel(c,8,165,224,94,accent);
        txt(12,174,!s.connected?"CHƯA KẾT NỐI ĐIỆN THOẠI":!s.hasProducerState?
            "BLE ĐÃ KẾT NỐI - CHỜ WAZE":s.navigationActive?"ĐỒNG HỒ - ĐANG CÓ LỘ TRÌNH":"ĐỒNG HỒ",
            assets::kTextSmall,accent,216,true);
        if(ms==INT64_MIN)txt(0,202,"--:--",assets::kTextLarge,colors::White,240,true);
        else {
            drawRobotDigit(c,42,202,time[0],colors::White);
            drawRobotDigit(c,82,202,time[1],colors::White);
            c.fillRect(117,213,6,6,colors::White);c.fillRect(117,229,6,6,colors::White);
            drawRobotDigit(c,132,202,time[3],colors::White);
            drawRobotDigit(c,172,202,time[4],colors::White);
        }
        angularPanel(c,12,267,216,30,accent);
        txt(15,271,date,assets::kTextSmall,colors::Amber,210,true);
        txt(0,302,fresh?"Chạm: trở về HUD":"Chờ điện thoại gửi dữ liệu",assets::kTextSmall,colors::Muted,240,true);
    } else {
        txt(10,2,time,assets::kTextSmall,colors::White,70);
        txt(105,2,settings.volume==0?"IM LẶNG":"",assets::kTextSmall,colors::Amber,90);
        txt(196,2,s.signalStale?"CHỜ":"BLE",assets::kTextSmall,s.signalStale?colors::Amber:colors::Green,40);
        txt(10,23,s.signalStale?"DỮ LIỆU GIÁN ĐOẠN":s.navigationActive?"DẪN ĐƯỜNG":"CHẠY TỰ DO",
            assets::kTextSmall,colors::Muted,220);
        c.line(10,44,230,44,colors::Muted);
        if(!fresh){
            txt(0,90,"--",assets::kTextLarge,colors::Muted,240,true);
            txt(0,152,"Chờ dữ liệu từ điện thoại",assets::kTextMedium,colors::Amber,240,true);
            txt(0,193,"Đang tự động kết nối lại…",assets::kTextSmall,colors::Muted,240,true);
        } else {
            char number[20]; std::snprintf(number,sizeof(number),"%d",std::clamp(s.speedKmh,0,999));
            const float scale=s.speedKmh>=100?1.0F:1.25F;
            c.fontTextScaled(10,53,number,assets::kNumberSpeedLimit,renderedOverspeedPhase_?colors::Red:colors::White,scale);
            txt(113,80,"km/h",assets::kTextSmall,colors::Muted);
            c.fillCircle(200,77,26,colors::Red);
            const bool night=settings.theme==UiTheme::Night || (settings.theme==UiTheme::Auto &&
                ms!=INT64_MIN && (local.tm_hour>=18 || local.tm_hour<6));
            c.fillCircle(200,77,21,night?colors::Panel:colors::White);
            if(s.speedLimitKmh>0)std::snprintf(number,sizeof(number),"%d",s.speedLimitKmh);
            else std::snprintf(number,sizeof(number),"?");
            c.fontTextScaled(177,66,number,assets::kNumberSpeedLimit3,night?colors::White:colors::Black,
                             s.speedLimitKmh>=100?0.5F:0.7F,46,true,true);
            txt(10,106,displayStreet(s),assets::kTextSmall,colors::Muted,220);
            AlertState sorted[8]{};
            const int count=collectSortedAlerts(s,sorted,8);
            auto alertRow=[&](const AlertState &a,int y,int extra){
                portraitCard(c,10,y,220,32,colors::Amber);
                drawAlertIcon(c,26,y+16,12,a,true);
                char distance[24];formatDistance(a.distanceM,distance,sizeof(distance));
                const int distanceWidth=c.fontTextWidth(distance,assets::kTextSmall);
                txt(44,y+3,alertKindLabel(a.kind),assets::kTextSmall,colors::Amber,178-distanceWidth);
                txt(225-distanceWidth,y+3,distance,assets::kTextSmall,alertDistanceColor(a.distanceM,colors::White));
                char detail[80]="";
                if(a.kind==AlertKind::TrafficJam && a.trafficDelayMinutes>=0)
                    std::snprintf(detail,sizeof(detail),"%s +%d phút",trafficSeverityLabel(a.trafficSeverity),a.trafficDelayMinutes);
                else if(extra>0)std::snprintf(detail,sizeof(detail),"+%d cảnh báo khác",extra);
                txt(44,y+16,detail,assets::kTextSmall,colors::Muted,176);
            };
            if(s.navigationActive){
                portraitCard(c,10,127,220,77,colors::Muted);
                if(s.maneuver!=Maneuver::None){
                    c.setTranslation(settings.offsetX,settings.offsetY-r.y+99);
                    drawManeuverIcon(c,s.maneuver,s.roundaboutExit,colors::Cyan);
                    c.setTranslation(settings.offsetX,settings.offsetY-r.y);
                    char distance[24];formatDistance(s.maneuverDistanceM,distance,sizeof(distance));
                    txt(88,129,distance,assets::kTextLarge,colors::White,137);
                    wrappedStreet(c,88,166,s.nextStreet.data(),137);
                }else txt(15,149,"Chờ hướng dẫn rẽ",assets::kTextMedium,colors::Muted,210,true);
                const bool haveLanes=s.laneCount>0;
                if(haveLanes){
                    portraitCard(c,10,210,220,35,colors::Muted);
                    int start=0, first=-1,last=-1;
                    const int total=std::min<int>(s.laneCount,kMaxLanes),visible=std::min(total,6);
                    for(int i=0;i<total;++i)if(s.lanes[i].selectedMask){if(first<0)first=i;last=i;}
                    if(first>=0)start=std::clamp((first+last)/2-visible/2,0,total-visible);
                    c.setTranslation(settings.offsetX,settings.offsetY-r.y+208);
                    for(int i=0;i<visible;++i)drawLane(c,27+i*(186/visible),std::min(12,186/visible/2),s.lanes[start+i],colors::Cyan);
                    c.setTranslation(settings.offsetX,settings.offsetY-r.y);
                    if(start>0)txt(11,215,"<",assets::kTextSmall,colors::Amber);
                    if(start+visible<total)txt(220,215,">",assets::kTextSmall,colors::Amber);
                }
                const bool haveAlert=count>0;
                if(haveAlert)alertRow(sorted[0],haveLanes?249:210,count-1);
                const int etaY=haveLanes&&haveAlert?286:250;
                char remaining[72], dist[24];
                if(s.remainingMeters>0 && s.remainingKm<1)std::snprintf(dist,sizeof(dist),"%d m",s.remainingMeters);
                else std::snprintf(dist,sizeof(dist),"%.1f km",s.remainingKm);
                if(haveLanes&&haveAlert){
                    std::snprintf(remaining,sizeof(remaining),"Đến %s - %s - %d ph",s.eta[0]?s.eta.data():"--:--",dist,s.remainingMinutes);
                    txt(10,etaY,remaining,assets::kTextSmall,colors::White,220,true);
                }else{
                    portraitCard(c,10,etaY,220,39,colors::Muted);
                    txt(17,etaY+1,"ĐẾN NƠI",assets::kTextSmall,colors::Muted,90);
                    txt(17,etaY+16,s.eta[0]?s.eta.data():"--:--",assets::kTextMedium,colors::White,90);
                    std::snprintf(remaining,sizeof(remaining),"%s - %d ph",dist,s.remainingMinutes);
                    txt(104,etaY+12,remaining,assets::kTextSmall,colors::Cyan,120,true);
                }
            }else{
                txt(0,133,date,assets::kTextSmall,colors::Amber,240,true);
                if(count>0){for(int i=0;i<std::min(count,3);++i)alertRow(sorted[i],165+i*40,i==2?count-3:0);}
                else{
                    portraitCard(c,10,168,220,109,colors::Muted);
                    txt(16,173,"HÀNH TRÌNH",assets::kTextSmall,colors::Muted,208);
                    const auto trip=trip_.display();char summary[64];
                    std::snprintf(summary,sizeof(summary),"%d.%d km - %d phút",trip.distanceTenthsKm/10,trip.distanceTenthsKm%10,trip.minutes);
                    txt(16,200,summary,assets::kTextMedium,colors::Cyan,208);
                    std::snprintf(summary,sizeof(summary),"MAX %d - TB %d km/h",trip.maxKmh,trip.avgKmh);
                    txt(16,239,summary,assets::kTextSmall,colors::White,208);
                }
            }
        }
        txt(0,302,"Chạm: giờ - Giữ: thống kê",assets::kTextSmall,colors::Muted,240,true);
        if(renderedOverspeedPhase_){c.fillRect(0,0,240,3,colors::Red);c.fillRect(0,317,240,3,colors::Red);
            c.fillRect(0,0,3,320,colors::Red);c.fillRect(237,0,3,320,colors::Red);}
    }
    if(volumeUntilMs_!=0)renderVolume(c,{0,0,240,320},240,320,settings);
}
