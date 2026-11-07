// Review-only SVG mockups; does not compile into firmware.
const fs = require('fs');
const path = require('path');
const sharp = require('/Users/bugatino/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/sharp');
const out = path.resolve(__dirname, '../docs/portrait-review');
fs.mkdirSync(out, {recursive:true});
const cyan='#54d8ff', amber='#ffc44b', white='#f1f6fb', muted='#93a5b7', green='#72e5a6', red='#ff6868';
const esc=s=>String(s).replaceAll('&','&amp;').replaceAll('<','&lt;');
const text=(x,y,s,size=12,color=white,anchor='start',weight=600)=>`<text x="${x}" y="${y}" font-family="Arial, sans-serif" font-size="${size}" font-weight="${weight}" fill="${color}" text-anchor="${anchor}">${esc(s)}</text>`;
const rect=(x,y,w,h,color='#102030',stroke='#263c50',r=8)=>`<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${r}" fill="${color}" stroke="${stroke}"/>`;
const line=(x,y,x2,y2,c=cyan,w=2)=>`<path d="M${x} ${y}L${x2} ${y2}" fill="none" stroke="${c}" stroke-width="${w}"/>`;
const shell=(body,border='#263c50')=>`<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="240" height="320" viewBox="0 0 240 320"><rect width="240" height="320" fill="#070e17"/><rect x="1" y="1" width="238" height="318" rx="8" fill="none" stroke="${border}"/>${body}</svg>`;
const header=(state='DẪN ĐƯỜNG',status='● BLE',color=green)=>text(10,17,'16:50',12)+text(230,17,status,10,color,'end')+text(10,36,state,10,muted)+line(10,43,230,43,'#263c50',1);
const sign=(value=60,cx=201,cy=76)=>`<circle cx="${cx}" cy="${cy}" r="25" fill="#f6f8fa" stroke="${red}" stroke-width="5"/>`+text(cx,cy+8,value,24,'#111923','middle',800);
const speed=(v=48,limit=60,over=false)=>text(12,92,v,49,over?red:white,'start',800)+text(87,90,'km/h',11,muted)+sign(limit);
const street=()=>text(12,117,'Đang đi: Đường Lê Lợi',11,muted);
const arrow=(kind='left',cx=43,cy=161)=>{
  if(kind==='round') return `<circle cx="${cx}" cy="${cy}" r="20" stroke="${cyan}" stroke-width="8" fill="none"/>`+line(cx+19,cy-5,cx+30,cy-19,cyan,8)+text(cx,cy+5,'3',13,white,'middle');
  const d=kind==='right'?`M${cx-10} ${cy+23}V${cy-14}H${cx+22}m-13 -13l13 13 -13 13`:kind==='straight'?`M${cx} ${cy+23}V${cy-23}m-14 14l14 -14 14 14`:`M${cx+10} ${cy+23}V${cy-14}H${cx-22}m13 -13l-13 13 13 13`;
  return `<path d="${d}" stroke="${cyan}" stroke-width="8" fill="none" stroke-linejoin="round"/>`;
};
const maneuver=(kind='left',distance='350 m',road='Võ Văn Kiệt')=>rect(10,127,220,76)+arrow(kind)+text(88,156,distance,26)+text(88,180,road,12)+text(88,194,kind==='round'?'Lối ra số 3':'Tiếp theo',10,muted);
const lanes=(n=4)=>rect(10,210,220,31)+Array.from({length:n},(_,i)=>text(31+i*(178/Math.max(1,n-1)),232,i===0?'↰':'↑',23,i===1||i===2?cyan:'#506071','middle')).join('');
const alert=(label='Camera tốc độ',dist='300 m',y=211,c=amber)=>rect(10,y,220,31,'#221e13','#65502a')+text(19,y+20,'!',15,c)+text(36,y+20,label,12,c)+text(220,y+20,dist,12,c,'end');
const eta=()=>rect(10,248,220,36)+text(18,261,'ĐẾN NƠI',9,muted)+text(18,277,'17:15',16)+text(222,261,'12,4 km',12,cyan,'end')+text(222,277,'25 phút',12,white,'end');
const footer=(s='Chạm: đồng hồ  ·  Vuốt: âm lượng')=>text(120,306,s,10,muted,'middle',400);
const nav=(extra=lanes(),kind='left',dist='350 m',road='Võ Văn Kiệt',v=48,limit=60)=>header()+speed(v,limit,v>limit)+street()+maneuver(kind,dist,road)+extra+eta()+footer();
const trip=()=>rect(10,174,220,99)+text(20,193,'HÀNH TRÌNH',10,muted)+text(20,221,'12,4 km',23,cyan)+text(220,221,'25 phút',17,white,'end')+text(20,254,'MAX 62',13,amber)+text(220,254,'TB 30 km/h',13,green,'end');
const free=(extra='')=>header('CHẠY TỰ DO')+speed()+street()+text(120,149,'THỨ BẢY · 07/11 · ÂL 29/09',11,amber,'middle')+(extra||trip())+footer();
const imageData=name=>'data:image/png;base64,'+fs.readFileSync(path.resolve(__dirname,'../waze-hud/main/assets/'+name)).toString('base64');
const digit=(x,y,ch)=>{
 const masks=[63,6,91,79,102,109,125,7,127,111]; const m=masks[Number(ch)]||0;
 const bars=[[3,0,19,5],[21,3,5,18],[21,25,5,18],[3,41,19,5],[0,25,5,18],[0,3,5,18],[3,20,19,5]];
 return bars.map(([a,b,w,h],i)=>(m&(1<<i))?`<rect x="${x+a}" y="${y+b}" width="${w}" height="${h}" fill="${white}"/>`:'').join('');
};
const clock=(bee=false,unsynced=false,waiting=false,active=false)=>{
 const c=bee?amber:cyan;
 let b=`<svg x="0" y="0" width="240" height="164" viewBox="0 0 320 164" preserveAspectRatio="xMidYMid slice"><image width="320" height="240" xlink:href="${imageData(bee?'clock_bumblebee.png':'clock_optimus.png')}"/></svg>`;
 b+=`<path d="M18 167H222L231 178V250L221 261H19L9 250V178Z" fill="#08121d" stroke="${c}" stroke-width="2"/>`;
 b+=text(120,185,waiting?'BLE ĐÃ KẾT NỐI · CHỜ DỮ LIỆU':active?'ĐỒNG HỒ · ĐANG CÓ LỘ TRÌNH':'CHƯA KẾT NỐI ĐIỆN THOẠI',9,c,'middle');
 b+=unsynced?text(120,235,'--:--',44,white,'middle'):digit(40,200,'1')+digit(75,200,'6')+rect(111,211,5,5,white,white,0)+rect(111,229,5,5,white,white,0)+digit(126,200,'5')+digit(161,200,'0');
 b+=rect(13,268,214,29,'#0d1722',c,3)+text(120,287,unsynced?'Chưa đồng bộ giờ':'T7 · 07/11 · ÂL 29/09',13,amber,'middle');
 return b+text(120,313,active?'Chạm để trở lại dẫn đường':'Chạm để xem HUD',10,muted,'middle');
};
const items=[];
const add=(id,title,body,note)=>items.push({id,title,body,note});
add('01','Khởi động · chưa có giờ',clock(false,true),'Không hiển thị ngày/giờ giả sau mất nguồn.');
add('02','Đồng hồ · Optimus',clock(),'Nền cơ khí, giờ segment, lịch âm; random mỗi lần cấp nguồn.');
add('03','Đồng hồ · Bumblebee',clock(true),'Cùng bố cục; đổi màu viền theo nhân vật.');
add('04','BLE kết nối · chờ dữ liệu',clock(false,false,true),'Tách trạng thái BLE khỏi trạng thái dữ liệu Waze.');
add('05','Chạy tự do · bình thường',free(),'Tốc độ xe là thông tin lớn nhất; ngày giờ chỉ xuất hiện một lần.');
add('06','Chạy tự do · nhiều cảnh báo',free(alert('Camera tốc độ','300 m',174)+alert('Công trường','600 m',211)+alert('Cảnh sát','900 m',248)),'Các cảnh báo còn lại gộp số lượng; không thu nhỏ chữ để nhét tất cả.');
add('07','Dẫn đường · không có làn',nav(alert()),'Hướng rẽ + khoảng cách lớn, tốc độ/giới hạn ở trên, ETA cố định.');
add('08','Dẫn đường · có làn',nav(),'Làn được chọn sáng cyan; các làn khác xám.');
add('09','Có làn và cảnh báo cùng lúc',header()+speed()+street()+maneuver()+lanes()+alert('Camera tốc độ','300 m',247)+text(120,295,'Đến 17:15 · 12,4 km · 25 phút',11,white,'middle')+text(120,312,'Chạm: đồng hồ',9,muted,'middle'),'Giữ cả làn, cảnh báo và ETA; không để làn đẩy cảnh báo biến mất.');
add('10','Nhiều làn · cửa sổ chọn',nav(lanes(6)+text(16,232,'‹',14,amber)+text(222,232,'›',14,amber)),'Ví dụ 8 làn: hiện 6 quanh làn được chọn, đánh dấu làn ngoài khung.');
add('11','Vòng xuyến · lối ra số 3',nav(alert('Cảnh sát','600 m'),'round','120 m'),'Các biến thể vòng xuyến dùng chung bố cục.');
add('12','Ra cao tốc / giữ phải',nav(lanes(),'right','800 m','Lối ra số 5'),'Tương tự cho giữ trái, chếch, rẽ gắt và quay đầu.');
add('13','Rẽ gần · ưu tiên thao tác',nav(alert('Đèn đỏ','80 m',211,red),'left','50 m'),'Khoảng cách sắp rẽ và cảnh báo gần được nhấn mạnh.');
add('14','Quá tốc độ',nav(alert('VƯỢT 8 km/h','GIẢM TỐC',211,red),'straight','1,2 km','Đường Nguyễn Trãi',68,60),'Viền đỏ/số tốc độ báo nguy; mute không tắt cảnh báo thị giác.');
add('15','Vùng cấm vượt',nav(alert('CẤM VƯỢT','1,2 km')),'Giữ thông tin cấm vượt khi có lộ trình.');
add('16','Kẹt xe · mức độ và trễ',nav(alert('Kẹt xe nặng · +18p','420 m')),'Mức độ kẹt xe và số phút trễ là hai trường riêng.');
add('17','Sắp tới đích',header()+speed()+street()+rect(10,127,220,76)+text(42,177,'⚑',44,cyan,'middle')+text(88,156,'40 m',26)+text(88,180,'Điểm đến',12)+alert('Điểm đến ở bên phải','')+rect(10,248,220,36)+text(18,261,'ĐẾN NƠI',9,muted)+text(18,277,'16:51',16)+text(222,261,'40 m',12,cyan,'end')+text(222,277,'1 phút',12,white,'end')+footer(),'Sắp tới đích; đến nơi dùng cùng vùng biểu tượng, khoảng cách về 0 khi dữ liệu xác nhận.');
add('18','Chưa biết giới hạn tốc độ',nav(alert(),'straight','1 km','Đường Lê Lợi',48,'—'),'Không diễn giải giới hạn chưa biết thành 0 hoặc đường vô hạn.');
add('19','Mất dữ liệu · không dùng số cũ',header('DỮ LIỆU GIÁN ĐOẠN','● CHỜ',amber)+text(120,123,'—',68,muted,'middle')+text(120,157,'Chờ điện thoại gửi dữ liệu',14,amber,'middle')+rect(16,185,208,76)+text(120,210,'Dữ liệu dẫn đường tạm dừng',12,white,'middle')+text(120,232,'Đang thử kết nối lại…',12,muted,'middle')+footer('Chạm: xem đồng hồ'),'Đề xuất trạng thái rõ ràng khi hết hạn dữ liệu; không giả vờ tốc độ vẫn hợp lệ.');
add('20','Thống kê chuyến đi · màn phụ',header('THỐNG KÊ CHUYẾN ĐI')+text(120,76,'Phiên chạy hiện tại',13,white,'middle')+rect(10,94,106,78)+rect(124,94,106,78)+rect(10,182,106,78)+rect(124,182,106,78)+text(63,116,'ĐÃ ĐI',10,muted,'middle')+text(63,147,'12,4 km',20,cyan,'middle')+text(177,116,'THỜI GIAN',10,muted,'middle')+text(177,147,'25 phút',20,white,'middle')+text(63,204,'CAO NHẤT',10,muted,'middle')+text(63,237,'62 km/h',18,amber,'middle')+text(177,204,'TRUNG BÌNH',10,muted,'middle')+text(177,237,'30 km/h',18,green,'middle')+footer('Chạm: trở về HUD'),'Không tự che ETA mỗi 5 phút; đường vào màn phụ cần bạn duyệt.');
add('21','Trạng thái thiết bị / BLE',header('THÔNG TIN THIẾT BỊ')+text(18,85,'WAZE HUD · ESP32-S3',16)+text(18,119,'BLE: Đã kết nối',14,green)+text(18,150,'Tín hiệu: −62 dBm',14)+text(18,181,'Hiển thị: Dọc 240 × 320',14)+text(18,212,'Loa: 50%',14)+text(18,243,'Giờ: Đã đồng bộ',14)+footer('Nút BOOT: trạng thái thiết bị'),'Không vẽ pin/GPS giả: board hiện chưa đọc pin, giao thức chưa có chất lượng GPS.');
const volume=(mute=false)=>nav()+rect(23,99,194,119,'#08131f',mute?amber:cyan)+text(120,125,mute?'ĐÃ TẮT TIẾNG':'ÂM LƯỢNG LOA',12,mute?amber:cyan,'middle')+text(120,162,mute?'0%':'50%',30,white,'middle')+rect(43,176,154,7,'#263c50','#263c50',3)+(mute?'':rect(43,176,77,7,cyan,cyan,3))+text(120,202,'Vuốt lên / xuống để chỉnh',10,muted,'middle');
add('22','Vuốt · âm lượng 50%',volume(),'Overlay ngắn khoảng 1,5 giây; vuốt đã nhận diện không phát sinh thao tác chạm.');
add('23','Vuốt · im lặng 0%',volume(true),'0% ngừng tiếng, vẫn hiện cảnh báo; lưu mức sau khi kết thúc thao tác.');
add('24','Đồng hồ khi đang dẫn đường',clock(false,false,false,true),'Chạm quay lại HUD; đề xuất tự bật HUD khi gần rẽ/cảnh báo quan trọng.');
add('25','Chế độ đêm',nav(alert('Camera tốc độ','300 m')),'Đề xuất giảm đèn nền, giữ màu cảnh báo; ảnh chỉ minh họa bố cục, không mô phỏng độ sáng vật lý.');
add('26','Có lộ trình · chưa có thao tác',header()+speed()+street()+rect(10,127,220,76)+text(120,157,'Chờ hướng dẫn rẽ',15,muted,'middle')+text(120,181,'Chưa có thao tác tiếp theo',11,muted,'middle')+alert()+eta()+footer(),'Ngày giờ chỉ ở header; không thay ô rẽ bằng thêm một đồng hồ hoặc đoán đi thẳng.');
add('27','Tên đường dài · xuống dòng',header()+speed()+street()+rect(10,127,220,76)+arrow()+text(88,150,'350 m',26)+text(88,175,'Cách Mạng Tháng Tám',11)+text(88,191,'Phường Bến Thành',11)+lanes()+eta()+footer(),'Ưu tiên hai dòng tĩnh trước khi cuộn; đầy đủ UTF-8 tiếng Việt.');
add('28','Nhiều cảnh báo khi dẫn đường',nav(alert('Camera tốc độ · +2','100 m')),'Hiện cảnh báo ưu tiên, số lượng còn lại; mức ưu tiên cần chốt trước triển khai.');

async function main(){
 for(const i of items){ const svg=shell(i.body,i.id==='14'?red:'#263c50');fs.writeFileSync(path.join(out,i.id+'.svg'),svg);await sharp(Buffer.from(svg)).png().toFile(path.join(out,i.id+'.png')); }
 for(let g=0;g<4;g++){
  const set=items.slice(g*7,g*7+7), tiles=[];
  for(let j=0;j<set.length;j++){
   const x=(j%4)*266+16,y=Math.floor(j/4)*375+55;
   tiles.push({input:path.join(out,set[j].id+'.png'),left:x,top:y});
   const label=`<svg width="256" height="42">${text(0,16,set[j].id+' · '+set[j].title,12,white)}</svg>`;
   tiles.push({input:Buffer.from(label),left:x,top:y+325});
  }
  await sharp({create:{width:1080,height:815,channels:4,background:'#0d1722'}}).composite(tiles).png().toFile(path.join(out,'sheet-'+(g+1)+'.png'));
 }
 const cards=items.map(i=>`<article id="s${i.id}"><h2>${i.id} · ${i.title}</h2><a href="${i.id}.png"><img src="${i.id}.png" width="240" height="320"></a><p>${i.note}</p></article>`).join('');
 fs.writeFileSync(path.join(out,'index.html'),`<!doctype html><html lang="vi"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Waze HUD · Review màn dọc</title><style>body{background:#0b121d;color:#e9f2fa;font:16px Arial;margin:28px auto;max-width:1180px;padding:0 18px}h1{font-size:28px}h2{font-size:16px;min-height:36px}p{line-height:1.5;color:#aabacb}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:20px}article{padding:16px;background:#122131;border:1px solid #263c50;border-radius:12px}img{display:block;margin:auto;image-rendering:pixelated}a{color:#54d8ff}</style><h1>Waze HUD · Thiết kế dọc 240 × 320</h1><p>Bản review bố cục ngày 07/11/2026 · Chưa triển khai firmware. Ảnh có dữ liệu minh họa; biểu tượng hướng rẽ/cảnh báo là ký hiệu bố cục, sẽ dùng asset HUD khi triển khai. Optimus/Bumblebee dùng lại ảnh nền đã duyệt; bố cục dọc sẽ cần asset riêng sau khi chốt.</p><p>Quy tắc: giờ hiện tại chỉ ở header; “ĐẾN NƠI” là ETA. Tốc độ xe lớn, biển giới hạn riêng. Làn/cảnh báo không đẩy nhau biến mất. Vuốt lên tăng, vuốt xuống giảm 0–100%; 0% tắt tiếng. Chạm đổi Đồng hồ ↔ HUD. Thống kê/thiết bị là màn phụ.</p><div class="grid">${cards}</div>`);
 console.log('Wrote '+items.length+' mockups, four contact sheets and HTML gallery to '+out);
}
main().catch(e=>{console.error(e);process.exit(1)});
