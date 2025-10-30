# ADC TRONG ESP8266

- **ADC : Analog To Digital Converter**

## Giới thiệu ADC

- Trong VDK, ADC là 1 thành phần quan trọng giúp chuyển đổi tín hiệu analog từ thế giới bên ngoài (ví dụ như tín hiệu từ cảm biến) thành tín hiệu số mà VDK có thể xử lý được.

## Thông số

- Trên ESP8266 NodeMCU chỉ có 1 chân ADC duy nhất là `ADC0 (A0)`

- `ADC` trong `ESP8266` có `độ phân giải 10 bit`, điện áp tham chiếu `Vref = 3.3V` => chỉ đọc được điện áp trong khoảng từ `0 - 3.3V`

## ADC trong VDK

### `analogRead();`

- Là API chuẩn dùng để đọc giá trị analog từ chân ADC (0 - 1023)

- Ví dụ: bài tập sử dụng biến trở xoay để đọc các giá trị analog của nó.

[Xem source code cơ bản tại đây](./ADCOnESP8266/src/main.cpp)
