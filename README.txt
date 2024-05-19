# iot-final

Trong qua trình học môn IOT, chúng tôi đã tạo ra 1 dự án với mô hình như sau: 
- 2 node điều khiển với cấu hình là: 
    + 1 Vi xử lý ESP32
    + 1 cảm biến nhiệt độ DHT11
    + 1 led báo hiệu 
    + và 1 số điện trở
- 2 node cảm biến này thực hiện truyền dữ liệu tới Broker Node-red bằng giao thức MQTT và truyền trực tiếp bằng giao thức CoAP
- Node-RED sẽ truyển dữ liệu tới Thingsboard bằng giao thức http
- Thingsboard sẽ chịu trách nhiệm hiển thị và xuất dữ liệu



- Về ESP32:
Trong file github này chúng tôi cung cấp file lập trình cho cặp vi xử lý ESP32(mqtt.ino và coap2.io)
Bằng cách setup hệ thống với mô hình phù hợp như trên. Người dùng tiến hành thay đổi 1 vài thông tin như ssid,password của Wifi, Địa chỉ IP, port sau đó tải code vào ESP32 là hoàn thành việc setup node cảm biển

Về Node-red:
- Các bạn có thể tìm hiểu về node-red tại đây: https://nodered.org/
- Các bạn setup như hình file json trong file EE4266-G24-Final

Về Thingsboard:
- Các bạn có thể tìm hiểu tại đây: "https://thingspeak.com/", "http://iottuonglai.com/thinkspeak-la-gi.html"
- Setup URL với mã token phù hợp để thực hiện dự án giống như của chúng tôi

Chúc các bạn thành công.