# WazeHUD cho ESP32-S3 2.8 inch (ES3C28P / ES3N28P)

HUD ô tô hiển thị dữ liệu dẫn đường Waze qua Bluetooth BLE (giao thức HLP/1 của Waze Mod), có **cảnh báo quá tốc độ bằng giọng nói** qua loa của board.

> [!NOTE]
> **Lời cảm ơn & Tôn trọng bản quyền tác giả (Credits & Respect):**
> Dự án này là phiên bản phát triển & tối ưu hóa mở rộng dựa trên mã nguồn gốc [WazeHUD của tác giả ShindouAris](https://github.com/ShindouAris/WazeHUD) và cộng đồng [WazeMod Vietnam](https://wazemod.io.vn). Phần giao diện, giao thức và cấu hình kế thừa từ bản CYD của [quangdo92](https://github.com/quangdo92/waze-hud). Xin trân trọng ghi nhận và cảm ơn công sức của các tác giả. Xin hãy luôn tôn trọng tác giả gốc và chia sẻ có trích nguồn. Giấy phép: GPL-3.0.

> [!IMPORTANT]
> Repo này **chỉ dành cho board ESP32-S3 2.8 inch ES3C28P / ES3N28P** (LCDWiki).
> Board **CYD ESP32-2432S028** (ESP32 thường) do [quangdo92](https://github.com/quangdo92/waze-hud) duy trì, vui lòng tải firmware CYD ở repo đó.
> **Không nạp firmware S3 lên board CYD và ngược lại.**

---

## 📥 Tải firmware

👉 **[GitHub Releases](https://github.com/haint14/Waze-Hud-ES3C28P-S3/releases/latest)**

| File | Dùng cho |
|---|---|
| `ES3C28P-BLE-<ngày>-<bản>.bin` | Dùng thật với Waze Mod qua Bluetooth BLE |
| `ES3C28P-MOCK-<ngày>-<bản>.bin` | Chạy thử giao diện + âm thanh, **không cần điện thoại** (12 kịch bản tự đổi mỗi 5 giây) |
| `SHA256.txt` | Mã kiểm tra toàn vẹn các file |

Đây là file **Factory All-in-One** (đã gồm bootloader, bảng phân vùng và firmware), nạp tại offset **`0x0`**.

## ⚡ Cách nạp

Cần Python và `esptool` (`pip install esptool`), hoặc dùng terminal của ESP-IDF.

```powershell
# Thay COM5 bằng cổng COM của board
python -m esptool --chip esp32s3 -p COM5 -b 460800 write_flash 0x0 ES3C28P-BLE-131026-b15.bin
```

- Chip: **ESP32-S3**, flash **16 MB**.
- Nếu board không tự vào chế độ nạp: giữ nút **BOOT**, nhấn **EN (RESET)** một lần, rồi thả BOOT.
- Cổng Type-C của board nối với USB nội của ESP32-S3. Windows 10 trở lên nhận cổng COM ảo, không cần driver riêng.
- Công cụ Web Flasher của [WazeMod](https://wazemod.io.vn/flash-firmware) có thể dùng được nếu hỗ trợ ESP32-S3 (chưa kiểm chứng với bản này).

---

## ✨ Tính năng

### 🕐 Màn hình chờ đồng hồ và lịch âm
- Khi chưa kết nối điện thoại, hoặc đã kết nối BLE nhưng Waze Mod chưa gửi dữ liệu, HUD chọn ngẫu nhiên nền Optimus hoặc Bumblebee lúc khởi động rồi hiển thị giờ, ngày dương và ngày âm thay cho hướng dẫn kết nối.
- Giờ được đồng bộ từ điện thoại qua HLP/1 và tiếp tục chạy khi Bluetooth ngắt. Sau khi rút nguồn, HUD cần kết nối điện thoại một lần để đồng bộ lại vì board không có RTC chạy pin.
- Ở cold boot chưa có mốc giờ, HUD hiện `--:--` và `Chưa đồng bộ giờ`; firmware không dùng build time hoặc giờ cũ làm giờ thật.
- Ảnh nền và thuật toán lịch âm được chuyển từ [bugatino/wazehud-esp32](https://github.com/bugatino/wazehud-esp32); bảng dữ liệu lịch âm có nguồn từ gói MIT `lunar` v2.0.0 của 6tail.

### 🔊 Cảnh báo quá tốc độ bằng giọng nói
- Từ `2.8.15-portrait.3`, âm lượng là điều khiển loa duy nhất: **0% = tắt tiếng**, lớn hơn 0% = bật cảnh báo. Bỏ công tắc phụ mặc định tắt của bản `.2` khiến loa không phát dù hiện 35%. Khi nâng cấp vẫn giữ mức âm lượng đã lưu. Firmware chưa có lệnh điều khiển tiếng điện thoại; cần tắt tiếng Waze trên điện thoại nếu chỉ muốn nghe một nguồn.
- Phát câu **"Bạn đã quá tốc độ"** qua loa khi tốc độ vượt giới hạn + ngưỡng (chỉnh trong app) liên tục **1,5 giây**.
- Lặp lại mỗi **4 giây** (tính từ lúc đọc xong) nếu vẫn quá tốc độ. Về tốc độ cho phép thì ngừng.
- Dùng codec **ES8311** + bộ khuếch đại của board. Nếu không tìm thấy codec, HUD vẫn chạy bình thường (chỉ ghi cảnh báo vào log).
- Có thể đổi giọng nói hoặc dùng tiếng bíp (xem mục *Tùy chỉnh âm thanh*).

### 🗣️ Lời đọc dẫn đường và cảnh báo (từ portrait.4)
- HUD đọc tiếng Việt trực tiếp từ dữ liệu HLP: chuẩn bị rẽ trong 500 m, nhắc rẽ trong 80 m, vòng xuyến/lối ra, giới hạn tốc độ hiện tại khi thay đổi, giới hạn tiếp theo trong 1 km và camera phía trước trong 600 m.
- Có lời đọc cho CSGT, tai nạn, đường đóng, công trường, đường sắt, cấm vượt, gờ giảm tốc, trường học, đèn giao thông, ùn tắc; biển khác dùng lời nhắc chung. Không đọc tên đường và không chuyển tiếp âm thanh của điện thoại.
- 52 đoạn giọng Linh tiếng Việt được nhúng offline; số giới hạn ghép từ 0–999. Mỗi sự kiện đọc một lần; rẽ gần được ưu tiên, câu cũ bị hủy khi đổi phiên/dữ liệu cũ hoặc không còn phù hợp. Quá tốc vẫn có nhịp nhắc riêng. Âm lượng 0% tắt toàn bộ lời đọc.
- `lan` là dữ liệu Waze phải gửi qua BLE. HUD hiện làn khi nhận được hướng hợp lệ kể cả cờ `nav` tắt; mask chọn sai chỉ bỏ tô màu, không bỏ cả hướng. Mảng `lan:[]` xóa làn ngay; trường thiếu vẫn có bộ lưu tạm giới hạn như trước. Trang trạng thái thiết bị và log `LAN wire/accepted/visible/nav` giúp phân biệt thiếu dữ liệu với lỗi vẽ.

### 🚗 Giao diện HUD tối ưu cho taplo ô tô
- Nền đen tuyệt đối, chống chói khi đặt trên taplo.
- Hàng trên cùng: **`dd/mm | hh:mm`** (ngày/tháng màu vàng, giờ màu trắng), đồng bộ từ điện thoại.
- Tốc độ xe và biển giới hạn tốc độ cỡ lớn. Khi quá tốc độ: viền đỏ nhấp nháy 2 Hz, số tốc độ nháy đỏ.
- Hướng rẽ, khoảng cách tới lượt rẽ, vòng xuyến (kèm lối ra), tối đa 10 làn đường với mũi tên chỉ hướng (kể cả rẽ chếch 45°).
- Tên đường hiện tại (có tiền tố `HT:` khi đang dẫn đường) và **tên đường kế tiếp trượt mượt ~30 FPS** dưới biển báo tốc độ.
- ETA, quãng đường và thời gian còn lại.
- Cảnh báo: camera phạt nguội, cảnh sát, tai nạn, kẹt xe (kèm mức độ và số phút trễ), công trường, đèn đỏ, vùng cấm vượt, các biển cấm/hiệu lệnh... hiển thị tối đa 4 thẻ khi chạy tự do.
- **Màu số mét cảnh báo 4 mức**: xanh lá (> 600 m), trắng (300–600 m), cam (100–300 m), đỏ (< 100 m).
- Chạy tiếng Việt có dấu (tự chạy chữ nếu tên đường quá dài).
- Chỉ vẽ lại đúng vùng thay đổi (dirty-region), không giật khung hình.

### 🌙 Độ sáng
- **Tự động theo giờ** (mặc định): 100% ban ngày (06:00–17:30), giảm dần lúc hoàng hôn (17:30–19:00), **20%** ban đêm (19:00–05:00), tăng dần lúc bình minh (05:00–06:00).
- Bật **"Cố định độ sáng"** trong app để dùng mức độ sáng chọn tay (10–100%).

### 🎨 Đảo màu từ app
- Mục **"Đảo ngược màu"** trong app bật/tắt chế độ đảo màu của màn hình (INVON/INVOFF) mà không cần nạp lại firmware. **Mặc định bật** cho board này. Nếu màn hình bị âm bản, hãy tắt đi.

---

## ⌨️ Nút BOOT (GPIO 0)

| Thao tác | Tác dụng |
|---|---|
| Nhấn 1 lần | Xoay màn hình 180° (tùy hướng cắm cáp) |
| Nhấn đúp | Bật / tắt lật gương (Mirror HUD) để chiếu hắt kính lái |
| Nhấn giữ | Hiện màn hình trạng thái thiết bị và BLE |

Các thiết lập xoay, lật gương được lưu vào NVS, không mất khi rút nguồn.

## ⚙️ Cấu hình từ Waze Mod

Khi Waze Mod hỗ trợ `device_config` (schema version 8), HUD gửi lên 11 thiết lập:

| Thiết lập | Giá trị | Ý nghĩa |
|---|---|---|
| Cố định độ sáng | Bật / Tắt | Bật để dùng độ sáng chọn tay, tắt để tự động theo giờ |
| Độ sáng | 10–100%, bước 5% | Chỉ chỉnh được khi bật "Cố định độ sáng" |
| Giao diện | Tự động / Ban ngày / Ban đêm | Màu giao diện biển báo |
| Đảo ngược màu | Bật / Tắt | Đảo cực tính màn hình (INVON/INVOFF) |
| Phản chiếu HUD | Bật / Tắt | Lật ngang để chiếu hắt kính lái |
| Xoay màn hình | Bật / Tắt | Xoay 180° theo hướng lắp |
| Hiển thị dọc | Bật / Tắt | Tắt: ngang 320×240; bật: dọc 240×320; độc lập với đảo 180° |
| Âm lượng loa | 0–100%, bước 5% | 0 = im lặng; vẫn hiện cảnh báo trên màn hình |
| Ngưỡng quá tốc | −10 đến +5 km/h | Bù vào giới hạn trước khi cảnh báo (áp dụng cả cho giọng nói) |
| Dịch ngang | −5 đến +5 px | Tinh chỉnh vị trí giao diện |
| Dịch dọc | −5 đến +5 px | Tinh chỉnh vị trí giao diện |

---

## 🔧 Phần cứng ES3C28P / ES3N28P

| Thành phần | Thông số / Chân kết nối |
|---|---|
| Vi điều khiển | ESP32-S3 (Xtensa LX7 lõi kép, 240 MHz), 16 MB flash, 8 MB PSRAM OPI |
| Màn hình LCD | ILI9341V 2.8 inch IPS, SPI @ 40 MHz, 240×320 (HUD dùng 320×240 landscape, xoay mềm) |
| Chân LCD | CS: GPIO 10 \| DC: GPIO 46 \| SCLK: GPIO 12 \| MOSI: GPIO 11 \| MISO: GPIO 13 |
| Reset LCD | Nối chung chân EN của ESP32-S3 (không dùng GPIO) |
| Đèn nền | GPIO 45 (LEDC PWM) |
| Loa / Âm thanh | Codec ES8311 (I2C: SDA 16, SCL 15; địa chỉ 0x18). I2S: MCLK 4, BCLK 5, LRCK 7, DOUT 8. Bật bộ khuếch đại: GPIO 1 (mức thấp) |
| Nút BOOT | GPIO 0 |
| Console / log | Cổng Type-C (USB Serial/JTAG) |

**Chưa hỗ trợ trong firmware**: đèn LED RGB (WS2812, GPIO 42), đo pin (GPIO 9), thẻ nhớ microSD, micro.

### Chạm và vuốt (ES3C28P có FT6336G)

- Chạm ngắn: đổi giữa đồng hồ và HUD. HUD tự chọn chạy tự do/dẫn đường theo dữ liệu điện thoại.
- Giữ khoảng 0,8 giây: mở/đóng thống kê chuyến đi; chạm để quay về HUD.
- Vuốt lên/xuống: tăng/giảm 5% mỗi khoảng 20 px, chặn ở 0–100%. Vuốt không phát sinh thao tác chạm.
- Hiện thanh âm lượng 1,5 giây; lưu NVS khi thả tay hoặc sau 1 giây không có bước chỉnh mới. 0% tắt cả tiếng đang phát, vẫn giữ cảnh báo thị giác.
- Khi mở đồng hồ/thống kê, giữ trang ít nhất 5 giây. Chỉ tự về HUD khi xuất hiện sự kiện khẩn **mới**: lượt rẽ cách ≤300 m, cảnh báo cách ≤100 m hoặc quá tốc độ (dữ liệu còn hợp lệ). Lượt rẽ cũ đang ở 0 m không làm bật lại HUD ngay sau khi chạm.
- Hướng dọc/ngang, đảo 180°, phản chiếu và âm lượng được lưu khi mất nguồn. App cần nhận lại schema sau thay đổi cấu hình bằng thao tác trên box.
- Màn dọc: trái là giờ/ngày và trạng thái loa, giữa là quãng đường/thời gian còn lại, phải là giờ đến/ETA (không có ETA thì hiện BLE). Bỏ ETA phía dưới và các dòng hướng dẫn thao tác.
- Hiện tối đa 2 cảnh báo khi có làn đường, 3 khi không có làn đường và 5 khi chạy tự do; `+N` cho biết số cảnh báo còn lại. Biển giới hạn sắp tới ghi “TỐC ĐỘ TIẾP THEO”.
- Số tốc độ/biển giới hạn dùng bitmap khử răng cưa ở đúng kích thước; làn đường dùng nét cong và đầu mũi tên đặc, nhánh ngang tách khỏi đầu đi thẳng. Hướng được chọn màu xanh, các nhánh còn lại màu trắng; tối đa 5 làn đang xem, ưu tiên vùng làn được chọn.
- Mã cảnh báo HLP `CAMERA_PHONE = 40` được hiển thị là **CAM PHẠT NGUỘI** theo tài liệu SDK; giữ nguyên mã giao thức và phân biệt camera tốc độ/đèn đỏ/dây an toàn.
- FT6336G dùng I²C 0x38 trên SDA16/SCL15, chia sẻ bus với ES8311. ES3N28P không có cảm ứng vẫn chạy HUD và cấu hình qua app; firmware ghi log nếu không tìm thấy chip cảm ứng.

Firmware `2.8.15-portrait.4` được kiểm tra bằng host preview, sanitizer, gesture/ánh xạ, cấu hình, PCM, task phát âm thanh với I2S/GPIO giả lập, bộ chọn lời đọc, giải mã giọng so với FFmpeg, decoder camera và dữ liệu JSON làn đi qua renderer thật, rồi build ESP-IDF. Lời đọc và làn đường từ Waze trên box vẫn cần thử thực tế.
File factory gộp nạp ở `0x0` có thể ghi đè vùng cấu hình NVS; cần chọn lại cấu hình sau khi nạp. Mất nguồn thông thường vẫn giữ cấu hình. Mặc định khi chưa có cấu hình là ngang, âm lượng 50%. Schema hiện có 11 mục, phiên bản 10; giữ lại mức âm lượng và các cài đặt cũ khi nâng cấp, bỏ qua khóa `hud_audio` cũ. Nếu từng bật phản chiếu để chữa lỗi chữ ngược ở màn dọc, hãy tắt phản chiếu sau khi nâng cấp để xem trực tiếp.
Tham chiếu thanh ghi cảm ứng: [Espressif FT5x06 driver](https://github.com/espressif/esp-bsp/blob/master/components/lcd_touch/esp_lcd_touch_ft5x06/esp_lcd_touch_ft5x06.c).

---

## 🛠️ Build từ mã nguồn

Yêu cầu: **ESP-IDF 5.5.x** (đã thử với 5.5.5), Windows PowerShell. Component `espressif/es8311` và `esp_lcd_ili9341` được tải tự động.

```powershell
cd waze-hud
.\build_es3c28p.ps1                 # -> dist\waze_hud_es3c28p_factory.bin
.\build_es3c28p.ps1 -Mock           # -> dist\waze_hud_es3c28p_mock_factory.bin
.\build_es3c28p.ps1 -Port COM5      # build, gộp và nạp luôn
```

Script tự dựng file Factory gộp (nạp tại `0x0`). Tham số `-IdfProfile` đổi đường dẫn profile PowerShell của ESP-IDF nếu bạn cài ở chỗ khác (mặc định `C:\Espressif\tools\Microsoft.v5.5.5.PowerShell_profile.ps1`).

### Tùy chỉnh âm thanh
Các tùy chọn nằm trong `waze-hud/sdkconfig.es3c28p.defaults` (xóa file `sdkconfig.es3c28p` rồi build lại để áp dụng):

| Tùy chọn | Mặc định | Ý nghĩa |
|---|---|---|
| `CONFIG_WAZE_HUD_AUDIO_VOLUME` | 49 | Âm lượng (1–100) |
| `CONFIG_WAZE_HUD_AUDIO_REPEAT_SEC` | 4 | Giây giữa hai lần cảnh báo (2–60) |
| `CONFIG_WAZE_HUD_AUDIO_USE_VOICE` | bật | Bật: đọc giọng nói; tắt: tiếng bíp-bíp-bíp |
| `CONFIG_WAZE_HUD_LCD_INVERT_COLOR` | bật | Mặc định đảo màu của màn hình |

**Đổi giọng nói:** ghi âm câu bạn muốn, xuất ra **WAV 16-bit** (ví dụ bằng Audacity), rồi:

```powershell
python waze-hud\tools\wav_to_voice_clip.py "audio\file.wav" --end 1.40
```

`--start` / `--end` (giây) dùng để cắt lấy một đoạn. Công cụ tự đổi sang 16 kHz mono, cắt khoảng lặng, chuẩn hóa âm lượng và ghi vào `waze-hud/main/audio/overspeed_voice.cpp`; sau đó build lại. File gốc "Bạn đã quá tốc độ" nằm trong thư mục `audio/`.

---

## 🔄 Theo dõi cập nhật từ bản CYD

Phần dùng chung (giao thức, hiển thị) được kế thừa từ [quangdo92/waze-hud](https://github.com/quangdo92/waze-hud). Để lấy cập nhật mới:

```powershell
git remote add upstream https://github.com/quangdo92/waze-hud
git fetch upstream
git merge upstream/CYD2.8-V2USB
```

Sau khi gộp, build lại cả hai board để chắc không vỡ (`.\build_es3c28p.ps1` cho S3, và build CYD bằng `sdkconfig.defaults` để kiểm tra).
