// Review-only UI revision from the user's real-device feedback.
// Does not modify or build the firmware.
const fs = require('fs');
const path = require('path');
const sharp = require('/Users/bugatino/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/sharp');
const out = path.resolve(__dirname, '../docs/portrait-review-v2');
fs.mkdirSync(out, {recursive:true});
const C = {bg:'#07101a',card:'#102131',edge:'#2c4355',text:'#f4f8fc',muted:'#a5b4c4',cyan:'#54dcff',amber:'#ffc558',red:'#ff646c',green:'#6fe2ac'};
const escape = s=>String(s).replaceAll('&','&amp;').replaceAll('<','&lt;');
const t=(x,y,s,size=12,color=C.text,align='start',weight=600)=>`<text x="${x}" y="${y}" fill="${color}" font-family="Arial, sans-serif" font-size="${size}" font-weight="${weight}" text-anchor="${align}">${escape(s)}</text>`;
const box=(x,y,w,h,fill=C.card,border=C.edge,r=7)=>`<rect x="${x}" y="${y}" width="${w}" height="${h}" fill="${fill}" stroke="${border}" rx="${r}"/>`;
const p=(d,color=C.cyan,width=4)=>`<path d="${d}" fill="none" stroke="${color}" stroke-width="${width}" stroke-linejoin="round" stroke-linecap="round"/>`;
const speaker=(x,y,volume=25)=>`<g transform="translate(${x} ${y})"><path d="M0 4H3L7 1V11L3 8H0Z" fill="${volume?C.muted:C.amber}"/>${volume?p('M10 3Q13 6 10 9',C.muted,1.4):p('M10 3L15 9M15 3L10 9',C.amber,1.4)}</g>`;
function header({nav=true,volume=25,time='10:58',eta='11:18',distance='9,7 km',duration='20 phút'}={}) {
    return t(8,18,time,16)+speaker(59,7,volume)+t(75,17,`${volume}%`,9,volume?C.muted:C.amber)+
      t(8,36,'07/11',12,C.muted)+
      (nav?t(134,17,distance,13,C.cyan,'middle')+t(134,36,duration,11,C.muted,'middle')+
        t(232,18,eta,16,C.text,'end')+t(232,36,'ETA',10,C.muted,'end'):
        t(232,18,'● BLE',11,C.green,'end'))+p('M8 44H232',C.edge,1);
}
function speed(v=22,limit=50,over=false) {
    return t(10,99,v,53,over?C.red:C.text,'start',800)+t(108,94,'km/h',11,C.muted)+
      `<circle cx="202" cy="78" r="25" fill="#f4f6f8" stroke="${C.red}" stroke-width="5"/>`+
      t(202,86,limit,24,'#111d29','middle',800)+t(10,115,'Ngô Gia Tự',12,C.muted);
}
function maneuver({round=false,exit=4,distance='350 m',road='Võ Văn Kiệt',line2=''}={}) {
    let a;
    if(round) {
        a=p('M47 190V184M28 178A24 24 0 1 1 69 175',C.cyan,6)+
          `<path d="M65 167L68 180L77 170Z" fill="${C.cyan}"/>`+
          `<text x="47" y="163" text-anchor="middle" dominant-baseline="central" font-family="Arial, sans-serif" font-size="25" font-weight="800" fill="${C.text}">${exit}</text>`;
    } else a=p('M54 185V157Q54 144 41 144H24',C.cyan,7)+p('M34 132L22 144L34 156',C.cyan,7);
    return box(8,123,224,77)+a+t(92,151,distance,26)+t(92,174,road,12)+
      (line2?t(92,190,line2,11,C.muted):'');
}
function lanes(n=4) {
    const color=(i)=>i===1||i===2?C.cyan:'#697d91';
    let b=box(8,205,224,42);
    const count=Math.min(n,5), gap=190/count, x0=25+gap/2;
    for(let i=0;i<count;i++) {
        const x=x0+i*gap,c=color(i);
        if(i===0) b+=p(`M${x} 239V228Q${x} 219 ${x-10} 219H${x-17}`,c,3.5)+p(`M${x-11} 213L${x-17} 219L${x-11} 225`,c,3.5);
        else if(i===count-1) b+=p(`M${x} 239V230Q${x} 223 ${x+7} 220L${x+13} 214`,c,3.5)+p(`M${x+5} 214H${x+13}V222`,c,3.5);
        else b+=p(`M${x} 239V213M${x-7} 220L${x} 213L${x+7} 220`,c,3.5);
        if(i===1||i===2)b+=`<rect x="${x-10}" y="242" width="20" height="2" fill="${C.cyan}" rx="1"/>`;
    }
    if(n>5)b+=t(13,232,'‹',17,C.amber)+t(225,232,'›',17,C.amber,'end');
    return b;
}
function icon(kind,x,y,value=80) {
    if(kind==='speed')return `<circle cx="${x}" cy="${y}" r="12" fill="#f4f6f8" stroke="${C.red}" stroke-width="2"/>`+t(x,y+4,value,11,'#111d29','middle',800);
    if(kind==='camera')return box(x-11,y-8,22,16,'none',C.amber,3)+`<circle cx="${x}" cy="${y}" r="4" fill="none" stroke="${C.amber}" stroke-width="2"/>`;
    return p(`M${x} ${y-11}L${x+12} ${y+10}H${x-12}Z`,C.amber,1.6)+t(x,y+6,'!',16,C.amber,'middle',800);
}
function alertRow(a,y,h=30,extra=0) {
    const col=a.danger?C.red:C.amber;
    return box(8,y,224,h,'#1d211f',a.danger?'#8b3b45':'#665132',5)+icon(a.kind||'warning',25,y+h/2,a.value)+
      t(43,y+12,a.label,10.5,col)+t(224,y+12,a.distance||'',10.5,col,'end')+
      t(43,y+25,a.detail||'',10.5,C.muted)+
      (extra?t(224,y+25,`+${extra}`,10.5,C.cyan,'end'):'');
}
const nextSpeed={kind:'speed',value:80,label:'TỐC ĐỘ TIẾP THEO',detail:'80 km/h',distance:'1,5 km'};
const camera={kind:'camera',label:'CAMERA TỐC ĐỘ',detail:'Phía trước',distance:'300 m'};
const works={label:'CÔNG TRƯỜNG',detail:'Đi chậm, chú ý',distance:'600 m'};
const traffic={label:'KẸT XE NẶNG',detail:'Chậm thêm 18 phút',distance:'900 m'};
function nav({withLanes=false,manyLanes=false,round=false,alerts=[nextSpeed,camera,works],volume=25,over=false}={}) {
    let b=header({volume})+speed(over?58:22,50,over)+maneuver({round,distance:round?'0 m':'350 m',road:round?'Ngô Gia Tự':'Võ Văn Kiệt'});
    if(withLanes)b+=lanes(manyLanes?8:4);
    const capacity=withLanes?2:3, start=withLanes?252:207, step=withLanes?33:36, h=withLanes?30:33;
    for(let i=0;i<Math.min(capacity,alerts.length);i++)b+=alertRow(alerts[i],start+i*step,h,i===capacity-1?Math.max(0,alerts.length-capacity):0);
    return b;
}
function free(alerts=[]) {
    let b=header({nav:false})+speed(50,80);
    if(alerts.length) {
      for(let i=0;i<Math.min(alerts.length,5);i++)b+=alertRow(alerts[i],126+i*38,34,i===4?Math.max(0,alerts.length-5):0);
    }else{
      b+=box(8,127,224,184)+t(20,149,'HÀNH TRÌNH',11,C.muted)+t(20,182,'12,4 km',25,C.cyan)+t(220,182,'25 phút',17,C.text,'end')+
      p('M20 202H220',C.edge,1)+t(20,224,'CAO NHẤT',10,C.muted)+t(220,224,'TRUNG BÌNH',10,C.muted,'end')+
      t(20,250,'62 km/h',16,C.amber)+t(220,250,'30 km/h',16,C.green,'end')+t(120,290,'T7 · ÂL 29/09',12,C.muted,'middle');
    }
    return b;
}
function clock(bee=false) {
    const data=fs.readFileSync(path.resolve(__dirname,'../waze-hud/main/assets/'+(bee?'clock_bumblebee.png':'clock_optimus.png'))).toString('base64');
    const color=bee?C.amber:C.cyan;
    const robotDigit=(x,y,n)=>{
      const masks=[63,6,91,79,102,109,125,7,127,111];
      const bars=[[3,0,24,6],[27,3,6,19],[27,25,6,19],[3,42,24,6],[0,25,6,19],[0,3,6,19],[3,21,24,6]];
      return bars.map(([a,b,w,h],i)=>(masks[n]&(1<<i))?`<rect x="${x+a}" y="${y+b}" width="${w}" height="${h}" fill="${C.text}"/>`:'').join('');
    };
    return `<svg width="240" height="164" viewBox="0 0 320 164" preserveAspectRatio="xMidYMid slice"><image xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="data:image/png;base64,${data}" width="320" height="240"/></svg>`+
      `<path d="M18 167H222L231 178V252L222 261H18L9 252V178Z" fill="#09131e" stroke="${color}" stroke-width="2"/>`+
      t(120,189,'ĐỒNG HỒ',10,color,'middle')+robotDigit(35,199,1)+robotDigit(75,199,0)+
      box(115,210,5,6,C.text,C.text,0)+box(115,228,5,6,C.text,C.text,0)+robotDigit(130,199,5)+robotDigit(170,199,8)+
      box(12,269,216,29,C.card,color,3)+t(120,288,'T7 - 07/11 - ÂL 29/09',13,C.amber,'middle');
}
const items=[
 ['01','Vòng xuyến · theo ảnh chạy thật',nav({round:true}),'Số 4 nằm giữa lòng vòng xuyến; tốc độ tiếp theo 80 không bị diễn giải là yêu cầu hạ xuống 80.'],
 ['02','Có làn · hai cảnh báo',nav({withLanes:true,alerts:[nextSpeed,camera]}),'Mũi tên làn cao hơn, nét cong và đầu mũi tên tròn; vùng cảnh báo giữ hai hàng.'],
 ['03','Không có làn · ba cảnh báo',nav({alerts:[nextSpeed,camera,works]}),'Bỏ toàn bộ ETA và hướng dẫn ở chân màn hình, lấy chỗ cho ba cảnh báo.'],
 ['04','Nhiều làn và nhiều cảnh báo',nav({withLanes:true,manyLanes:true,alerts:[nextSpeed,camera,works,traffic]}),'Giữ tối đa năm làn đủ lớn; hai thẻ cảnh báo, +2 báo số cảnh báo ngoài vùng nhìn.'],
 ['05','Chạy tự do · thống kê',free(),'Header trái là giờ, loa và ngày; bên phải mới hiện BLE khi không có ETA.'],
 ['06','Chạy tự do · nhiều cảnh báo',free([nextSpeed,camera,works,traffic,{label:'CẤM VƯỢT',detail:'Còn 1,2 km trong vùng',distance:''}]),'Dành tối đa năm hàng cho cảnh báo, không lặp ngày giờ giữa màn hình.'],
 ['07','Im lặng · hiển thị trên header',nav({withLanes:true,volume:0,alerts:[nextSpeed,camera]}),'Icon loa gạch chéo và 0%; cảnh báo thị giác vẫn còn.'],
 ['08','Quá tốc độ · giữ thông tin chuyến đi',nav({over:true,alerts:[{label:'VƯỢT GIỚI HẠN',detail:'Vượt 8 km/h',distance:'',danger:true},camera,works]}),'Đỏ dành cho cảnh báo hiện tại; tốc độ tiếp theo không bị tô như một yêu cầu giảm tốc.'],
 ['09','Đồng hồ · Optimus',clock(),'Bỏ chữ hướng dẫn ở đáy. Đây là bản minh họa bố cục, giờ và lịch sẽ là dữ liệu động.'],
 ['10','Đồng hồ · Bumblebee',clock(true),'Vẫn giữ cả hai nhân vật; không thay đổi quy tắc chọn nhân vật trong lượt review.'],
 ['11','Vuốt âm lượng · panel gọn',nav({withLanes:true})+box(35,126,170,73,'#07111c',C.cyan)+speaker(53,146,25)+t(174,159,'25%',26,C.text,'end')+box(54,176,132,5,C.edge,C.edge,2)+box(54,176,33,5,C.cyan,C.cyan,2),'Panel chỉ hiện mức âm lượng và thanh mức, không lặp hướng dẫn thao tác.'],
 ['12','Giới hạn hiện tại và sắp tới',header()+speed(50,50)+maneuver({road:'Ngô Gia Tự'})+alertRow(nextSpeed,207,33)+alertRow(camera,243,33)+alertRow(works,279,33),'Ví dụ: xe đang 50, biển hiện tại 50, biển sắp tới 80 ở 1,5 km. Ba khái niệm hiển thị riêng.'],
];
const svg=body=>`<svg xmlns="http://www.w3.org/2000/svg" width="240" height="320" viewBox="0 0 240 320"><rect width="240" height="320" fill="${C.bg}"/>${body}</svg>`;
async function main(){
 for(const [id,,body] of items){fs.writeFileSync(path.join(out,id+'.svg'),svg(body));await sharp(Buffer.from(svg(body))).png().toFile(path.join(out,id+'.png'));}
 for(let g=0;g<2;g++){
  const layers=[];
  for(let i=0;i<6;i++){
   const [id,title]=items[g*6+i],x=18+(i%3)*276,y=24+Math.floor(i/3)*366;
   layers.push({input:path.join(out,id+'.png'),left:x,top:y});
   layers.push({input:Buffer.from(`<svg width="260" height="30">${t(0,17,id+' · '+title,11)}</svg>`),left:x,top:y+324});
  }
  await sharp({create:{width:846,height:758,channels:4,background:'#0d1824'}}).composite(layers).png().toFile(path.join(out,'sheet-'+(g+1)+'.png'));
 }
 const cards=items.map(([id,title,,note])=>`<article><h2>${id} · ${title}</h2><a href="${id}.png"><img src="${id}.png" width="240" height="320"></a><p>${note}</p></article>`).join('');
 fs.writeFileSync(path.join(out,'index.html'),`<!doctype html><html lang="vi"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>HUD dọc · Review v2</title><style>body{background:#07101a;color:#f4f8fc;font:15px Arial;max-width:1150px;margin:30px auto;padding:0 20px}h1{font-size:26px}h2{font-size:16px;min-height:38px}p{color:#a5b4c4;line-height:1.6}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:20px}article{background:#102131;padding:18px;border-radius:12px}img{display:block;margin:auto}a{color:#54dcff}</style><h1>Waze HUD dọc · Review v2</h1><p>Ngày 07/11/2026 · Chỉ cập nhật demo sau phản hồi chạy thật. Chưa sửa firmware hoặc xuất .bin mới. Ảnh có kích thước 240×320; font và nét SVG được rasterize trực tiếp ở kích thước đích. Chất lượng font trên firmware chỉ được xác nhận khi thay bộ glyph thực và kiểm tra trên board.</p><p>Header: trái = giờ + loa/mức âm lượng, dưới = ngày; giữa = khoảng cách còn lại / thời gian còn lại; phải = giờ đến dự kiến / ETA. Khi không có ETA thì bên phải hiện trạng thái BLE. Bỏ tên chế độ và toàn bộ hướng dẫn cố định dưới màn hình. Đầu mũi tên làn lớn hơn, thân cong bo tròn.</p><p>Có làn: 2 cảnh báo + số lượng còn lại. Không có làn: 3 cảnh báo. Chạy tự do: tối đa 5 cảnh báo. Không tự xoay vòng cảnh báo khiến người lái phải đợi đọc.</p><div class="grid">${cards}</div></html>`);
 console.log(`Created ${items.length} review mockups at ${out}`);
}
main().catch(e=>{console.error(e);process.exit(1)});
