// Native 240x320 layout. All coordinates are screen coordinates; translation
// clips each composition into the renderer's bounded forty-row DMA buffer.
namespace {
void portraitCard(Canvas &c,int x,int y,int w,int h,uint16_t border){
    c.fillRect(x,y,w,h,colors::rgb565(16,32,48));
    drawCard(c,x,y,w,h,border==colors::Muted?colors::rgb565(45,67,85):border);
}
void wrappedStreet(Canvas &c,int x,int y,const char *value,int width){
    const auto &font=assets::kHudSmall;
    if(c.fontTextWidth(value,font)<=width){c.fontText(x,y,value,font,colors::White,width);return;}
    char first[164]{};int split=0;
    for(int i=0;value[i]&&i<160;++i){
        first[i]=value[i];first[i+1]=0;
        if(value[i]==' '&&c.fontTextWidth(first,font)<=width)split=i;
        if(c.fontTextWidth(first,font)>width)break;
    }
    if(split>0){first[split]=0;c.fontText(x,y,first,font,colors::White,width);
        c.fontText(x,y+16,value+split+1,font,colors::White,width);}
    else c.fontText(x,y,value,font,colors::White,width);
}
void angularPanel(Canvas &c,int x,int y,int w,int h,uint16_t color){
    c.fillRect(x+7,y,w-14,h,colors::Panel);c.fillRect(x,y+7,w,h-14,colors::Panel);
    const int xs[]={x+7,x+w-8,x+w-1,x+w-1,x+w-8,x+7,x,x,x+7};
    const int ys[]={y,y,y+7,y+h-8,y+h-1,y+h-1,y+h-8,y+7,y};
    for(int i=0;i<8;++i)c.line(xs[i],ys[i],xs[i+1],ys[i+1],color);
}
void speakerIcon(Canvas &c,int x,int y,bool audible){
    const auto color=audible?colors::Muted:colors::Amber;
    c.fillRect(x,y+4,3,6,color);
    for(int i=0;i<5;++i)c.line(x+3+i,y+4-i,x+3+i,y+9+i,color);
    if(audible){c.line(x+10,y+3,x+12,y+6,color);c.line(x+12,y+6,x+10,y+10,color);}
    else {c.line(x+10,y+3,x+15,y+10,color);c.line(x+15,y+3,x+10,y+10,color);}
}
}

void HudRenderer::renderVolume(Canvas &c,const Rect &r,int width,int height,const DeviceSettings &settings){
    const int x=(width-172)/2-r.x,y=(height-74)/2-r.y;
    portraitCard(c,x,y,172,74,colors::Cyan);
    speakerIcon(c,x+17,y+20,settings.hudAudioEnabled&&settings.volume>0);
    char volume[12];std::snprintf(volume,sizeof(volume),"%u%%",settings.volume);
    c.fontText(x+45,y+9,volume,assets::kTextLarge,colors::White,110,true);
    c.fillRect(x+20,y+54,132,5,colors::Muted);
    c.fillRect(x+20,y+54,132*settings.volume/100,5,colors::Cyan);
}

void HudRenderer::renderTripPage(Canvas &c,const Rect &r,int width,int height){
    c.clear(colors::rgb565(7,14,23));const auto trip=trip_.display();
    c.fontText(-r.x,12-r.y,"THỐNG KÊ CHUYẾN ĐI",assets::kHudHeader,colors::Cyan,width,true);
    const int top=height==320?86:50,cellW=(width-30)/2,cellH=height==320?78:70;
    char distance[24],duration[24],maximum[24],avg[24];
    std::snprintf(distance,sizeof(distance),"%d.%d km",trip.distanceTenthsKm/10,trip.distanceTenthsKm%10);
    std::snprintf(duration,sizeof(duration),"%d phút",trip.minutes);
    std::snprintf(maximum,sizeof(maximum),"%d km/h",trip.maxKmh);
    std::snprintf(avg,sizeof(avg),"%d km/h",trip.avgKmh);
    const char *labels[]={"ĐÃ ĐI","THỜI GIAN","CAO NHẤT","TRUNG BÌNH"};
    const char *values[]={distance,duration,maximum,avg};
    for(int i=0;i<4;++i){const int x=10+(i%2)*(cellW+10)-r.x,y=top+(i/2)*(cellH+10)-r.y;
        portraitCard(c,x,y,cellW,cellH,colors::Muted);
        c.fontText(x,y+8,labels[i],assets::kHudTiny,colors::Muted,cellW,true);
        c.fontText(x,y+32,values[i],assets::kHudHeader,i==0?colors::Cyan:colors::White,cellW,true);}
}

void HudRenderer::renderPortrait(Canvas &c,const Rect &r,const HudState &s,
                                 const DeviceSettings &settings,const SystemStatusSnapshot &sys){
    c.clear(colors::rgb565(7,14,23));
    auto txt=[&](int x,int y,const char *v,const assets::BitmapFont &f,uint16_t col,int w=-1,bool center=false){
        c.fontText(x,y,v,f,col,w,center);
    };
    const bool fresh=s.connected&&s.hasProducerState&&!s.signalStale;
    const auto accent=bumblebeeClock_?colors::Amber:colors::Cyan;
    const int64_t ms=localClockMillis(s);tm local{};
    char time[8]="--:--",dayMonth[8]="--/--",date[56]="Chưa đồng bộ giờ";
    if(ms!=INT64_MIN){const time_t sec=ms/1000;gmtime_r(&sec,&local);
        std::snprintf(time,sizeof(time),"%02d:%02d",local.tm_hour,local.tm_min);
        std::strftime(dayMonth,sizeof(dayMonth),"%d/%m",&local);
        const char *days[]={"CN","T2","T3","T4","T5","T6","T7"};
        lunar_date_t lunar{};
        if(lunar_from_unix(currentClockUnixSeconds(s),&lunar))
            std::snprintf(date,sizeof(date),"%s - %02d/%02d - ÂL %02d/%02d%s",days[local.tm_wday],local.tm_mday,
                local.tm_mon+1,lunar.day,lunar.month,lunar.leap_month?"n":"");
        else std::snprintf(date,sizeof(date),"%s - %s",days[local.tm_wday],dayMonth);
    }
    if(sys.visible){
        txt(0,12,"TRẠNG THÁI THIẾT BỊ",assets::kHudHeader,colors::Cyan,240,true);
        txt(15,58,"WAZE HUD - ESP32-S3",assets::kHudHeader,colors::White,210);
        char detail[80];
        std::snprintf(detail,sizeof(detail),"BLE: %s",sys.bleConnected?"Đã kết nối":"Chưa kết nối");
        txt(15,100,detail,assets::kHudHeader,sys.bleConnected?colors::Green:colors::Amber,210);
        std::snprintf(detail,sizeof(detail),"Tín hiệu: %d dBm",sys.bleRssiDbm);
        if(sys.bleConnected)txt(15,134,detail,assets::kHudHeader,colors::White,210);
        txt(15,168,"Hiển thị: Dọc 240 x 320",assets::kHudSmall,colors::White,210);
        std::snprintf(detail,sizeof(detail),"Loa HUD: %s - %u%%",settings.hudAudioEnabled?"Bật":"Tắt",settings.volume);
        txt(15,203,detail,assets::kHudSmall,colors::White,210);
        txt(15,240,ms==INT64_MIN?"Giờ: Chưa đồng bộ":"Giờ: Đã đồng bộ",assets::kHudSmall,colors::Muted,210);return;
    }
    if(page_==2)renderTripPage(c,{0,0,240,320},240,320);
    else if(page_==1||((!s.connected||!s.hasProducerState)&&!s.signalStale)){
        const auto *bg=reinterpret_cast<const uint16_t *>(bumblebeeClock_?clockBumblebeeStart:clockOptimusStart);
        for(int y=std::max<int>(0,r.y-settings.offsetY);y<std::min<int>(164,r.y+r.height-settings.offsetY);++y)
            for(int x=0;x<240;++x)c.pixel(x,y,bg[y*320+(x*320/240)]);
        angularPanel(c,8,165,224,94,accent);
        txt(12,175,!s.connected?"CHƯA KẾT NỐI ĐIỆN THOẠI":!s.hasProducerState?"BLE KẾT NỐI - CHỜ WAZE":"ĐỒNG HỒ",assets::kHudTiny,accent,216,true);
        if(ms==INT64_MIN)txt(0,202,"--:--",assets::kTextLarge,colors::White,240,true);
        else {drawRobotDigit(c,42,202,time[0],colors::White);drawRobotDigit(c,82,202,time[1],colors::White);
            c.fillRect(117,213,6,6,colors::White);c.fillRect(117,229,6,6,colors::White);
            drawRobotDigit(c,132,202,time[3],colors::White);drawRobotDigit(c,172,202,time[4],colors::White);}
        angularPanel(c,12,267,216,30,accent);txt(15,272,date,assets::kHudSmall,colors::Amber,210,true);
    }else{
        txt(8,0,time,assets::kHudHeader,colors::White,50);
        const bool audible=settings.hudAudioEnabled&&settings.volume>0;
        speakerIcon(c,59,4,audible);char volume[8];std::snprintf(volume,sizeof(volume),"%u%%",settings.volume);
        txt(76,4,volume,assets::kHudTiny,audible?colors::Muted:colors::Amber,27);
        txt(8,22,dayMonth,assets::kHudSmall,colors::Muted,65);
        const bool hasEta=fresh&&s.navigationActive&&s.eta[0];
        if(fresh&&s.navigationActive){
            char distance[24],duration[24];
            if(s.remainingKm>=1)std::snprintf(distance,sizeof(distance),"%.1f km",s.remainingKm);
            else std::snprintf(distance,sizeof(distance),"%d m",std::max(0,s.remainingMeters));
            if(s.remainingMinutes>=60)std::snprintf(duration,sizeof(duration),"%dh %02dp",s.remainingMinutes/60,s.remainingMinutes%60);
            else std::snprintf(duration,sizeof(duration),"%d phút",std::max(0,s.remainingMinutes));
            txt(103,1,distance,assets::kHudSmall,colors::Cyan,62,true);
            txt(103,23,duration,assets::kHudTiny,colors::Muted,62,true);
        }
        if(hasEta){txt(180,0,s.eta.data(),assets::kHudHeader,colors::White,52,true);txt(180,23,"ETA",assets::kHudTiny,colors::Muted,52,true);}
        else txt(188,3,s.signalStale?"CHỜ":"BLE",assets::kHudSmall,s.signalStale?colors::Amber:colors::Green,44,true);
        c.line(8,44,232,44,colors::rgb565(45,67,85));
        if(!fresh){txt(0,92,"--",assets::kTextLarge,colors::Muted,240,true);
            txt(0,155,"Chờ dữ liệu từ điện thoại",assets::kHudHeader,colors::Amber,240,true);
            txt(0,193,"Đang tự động kết nối lại...",assets::kHudSmall,colors::Muted,240,true);
        }else{
            char number[20];std::snprintf(number,sizeof(number),"%d",std::clamp(s.speedKmh,0,999));
            txt(10,48,number,s.speedKmh>=100?assets::kHudSpeed3:assets::kHudSpeed,
                renderedOverspeedPhase_?colors::Red:colors::White,105);
            txt(117,83,"km/h",assets::kHudTiny,colors::Muted,40);
            const bool night=settings.theme==UiTheme::Night||(settings.theme==UiTheme::Auto&&ms!=INT64_MIN&&(local.tm_hour>=18||local.tm_hour<6));
            c.fillCircle(202,78,26,colors::Red);c.fillCircle(202,78,21,night?colors::Panel:colors::White);
            if(s.speedLimitKmh>0)std::snprintf(number,sizeof(number),"%d",s.speedLimitKmh);else std::snprintf(number,sizeof(number),"?");
            drawNativeCentered(c,202,78,number,s.speedLimitKmh>=100?assets::kHudLimit3:assets::kHudLimit,night?colors::White:colors::Black);
            txt(10,106,displayStreet(s),assets::kHudSmall,colors::Muted,220);
            AlertState sorted[8]{};const int count=collectSortedAlerts(s,sorted,8);
            auto alertRow=[&](const AlertState &a,int y,int height,int extra){
                portraitCard(c,8,y,224,height,colors::Amber);
                if(a.kind==AlertKind::SpeedDrop&&a.valueKmh>0){
                    c.fillCircle(25,y+height/2,12,colors::Red);c.fillCircle(25,y+height/2,10,colors::White);
                    char limit[12];std::snprintf(limit,sizeof(limit),"%d",a.valueKmh);
                    drawNativeCentered(c,25,y+height/2,limit,assets::kHudTiny,colors::Black);
                }else drawAlertIcon(c,25,y+height/2,12,a,true);
                char distance[24];formatDistance(a.distanceM,distance,sizeof(distance));
                const int dw=c.fontTextWidth(distance,assets::kHudTiny);
                txt(43,y+1,alertKindLabel(a.kind),assets::kHudTiny,colors::Amber,178-dw);
                txt(225-dw,y+1,distance,assets::kHudTiny,alertDistanceColor(a.distanceM,colors::White));
                char detail[96]="";
                if(a.kind==AlertKind::SpeedDrop&&a.valueKmh>0)std::snprintf(detail,sizeof(detail),"%d km/h",a.valueKmh);
                else if(a.kind==AlertKind::TrafficJam&&a.trafficDelayMinutes>=0)
                    std::snprintf(detail,sizeof(detail),"%s +%d phút",trafficSeverityLabel(a.trafficSeverity),a.trafficDelayMinutes);
                if(extra>0){char more[12];std::snprintf(more,sizeof(more),"+%d",extra);txt(201,y+15,more,assets::kHudTiny,colors::Cyan,24,true);}
                txt(43,y+15,detail,assets::kHudTiny,colors::Muted,extra>0?152:178);
            };
            if(s.navigationActive){
                portraitCard(c,8,123,224,77,colors::Muted);
                if(s.maneuver!=Maneuver::None){
                    c.setTranslation(settings.offsetX,settings.offsetY-r.y+97);
                    drawManeuverIcon(c,s.maneuver,s.roundaboutExit,colors::Cyan);
                    c.setTranslation(settings.offsetX,settings.offsetY-r.y);
                    char distance[24];formatDistance(s.maneuverDistanceM,distance,sizeof(distance));
                    txt(91,127,distance,assets::kTextLarge,colors::White,136);
                    wrappedStreet(c,91,164,s.nextStreet.data(),136);
                }else txt(14,148,"Chờ hướng dẫn rẽ",assets::kHudHeader,colors::Muted,212,true);
                const bool haveLanes=s.laneCount>0;
                if(haveLanes){
                    portraitCard(c,8,205,224,42,colors::Muted);
                    const int total=std::min<int>(s.laneCount,kMaxLanes),visible=std::min(total,5);
                    int first=-1,last=-1,start=0;
                    for(int i=0;i<total;++i)if(s.lanes[i].selectedMask){if(first<0)first=i;last=i;}
                    if(first>=0)start=std::clamp((first+last)/2-visible/2,0,total-visible);
                    const int spacing=200/visible;
                    for(int i=0;i<visible;++i){const auto &lane=s.lanes[start+i];const int x=20+spacing*i+spacing/2-18;
                        c.alphaMask(x,208,assets::kPortraitLaneStem,lane.selectedMask?colors::Cyan:colors::Muted);
                        for(int b=0;b<8;++b)if(lane.directionMask&(1U<<b))c.alphaMask(x,208,assets::kPortraitLaneBranches[b],lane.selectedMask&(1U<<b)?colors::Cyan:colors::Muted);
                        if(lane.selectedMask)c.fillRect(x+8,243,20,2,colors::Cyan);
                    }
                    if(start>0)txt(9,217,"<",assets::kHudSmall,colors::Amber);
                    if(start+visible<total)txt(222,217,">",assets::kHudSmall,colors::Amber);
                }
                const int capacity=haveLanes?2:3,top=haveLanes?252:207,step=haveLanes?33:36,height=haveLanes?30:33;
                for(int i=0;i<std::min(count,capacity);++i)alertRow(sorted[i],top+i*step,height,i==capacity-1?count-capacity:0);
            }else if(count>0){
                for(int i=0;i<std::min(count,5);++i)alertRow(sorted[i],126+i*38,34,i==4?count-5:0);
            }else{
                portraitCard(c,8,127,224,184,colors::Muted);const auto trip=trip_.display();char value[48];
                txt(20,138,"HÀNH TRÌNH",assets::kHudSmall,colors::Muted,200);
                std::snprintf(value,sizeof(value),"%d.%d km",trip.distanceTenthsKm/10,trip.distanceTenthsKm%10);txt(20,165,value,assets::kTextLarge,colors::Cyan,120);
                std::snprintf(value,sizeof(value),"%d phút",trip.minutes);txt(140,171,value,assets::kHudHeader,colors::White,80,true);
                c.line(20,201,220,201,colors::rgb565(45,67,85));
                txt(20,212,"CAO NHẤT",assets::kHudTiny,colors::Muted,90);txt(130,212,"TRUNG BÌNH",assets::kHudTiny,colors::Muted,90,true);
                std::snprintf(value,sizeof(value),"%d km/h",trip.maxKmh);txt(20,238,value,assets::kHudHeader,colors::Amber,90);
                std::snprintf(value,sizeof(value),"%d km/h",trip.avgKmh);txt(130,238,value,assets::kHudHeader,colors::Green,90,true);
                txt(12,283,date,assets::kHudTiny,colors::Muted,216,true);
            }
        }
        if(renderedOverspeedPhase_){c.fillRect(0,0,240,3,colors::Red);c.fillRect(0,317,240,3,colors::Red);c.fillRect(0,0,3,320,colors::Red);c.fillRect(237,0,3,320,colors::Red);}
    }
    if(volumeUntilMs_!=0)renderVolume(c,{0,0,240,320},240,320,settings);
}
