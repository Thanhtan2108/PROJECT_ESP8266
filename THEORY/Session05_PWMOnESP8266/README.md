# PWM TRONG ESP8266

## Giới thiệu về PWM

- Là kỹ thuật điều chế độ rộng xung

- Dùng để điều khiển độ sáng LED, tốc độ động cơ, âm thanh, v.v.

- Để có thể điều chế mức điện áp từ 0 - 3.3V trên chân GPIO

## Cơ chế của PWM

- PWM hoạt động bằng cách thay đổi tỷ lệ thời gian xung HIGH (duty cycle) so với thời gian chu kỳ tổng thể (period).

- Giá trị duty cycle giao động từ 0% đến 100%:
  
  - 0%: Tín hiệu luôn ở mức LOW(0VDC)

  - 100%: tín hiệu luôn ở mức HIGH (3.3VDC đối với ESP8266)

- Độ phân giải 10-bit, nên giá trị duty cycle có thể dao động từ **0 - 1023**.

- PWM :

  - 0 - 100%

  - 0 -y-> 255 (giá trị analog)

  - 0 -x-> 3.3V (Vref)

- Ví dụ: muốn điều chỉnh Vref là 2V thì

  - lập tỷ số: y/2 = 255/3.3 => y

- Điều chỉnh độ rộng xung theo duty cycle

| Duty (%) | Giá trị analogWrite (với RANGE = 1023) |
| -------- | -------------------------------------- |
| 0%       | 0                                      |
| 10%      | 102                                    |
| 25%      | 256                                    |
| 50%      | 512                                    |
| 75%      | 767                                    |
| 100%     | 1023                                   |

- Công thức tính giá trị analog tương ứng với duty cycle để điều khiển theo duty cycle bằng value analog và truyền vào `analogWrite()`

```text
value = (duty_percent/100​) × 1023
```

- Ví dụ: muốn LED sáng 50%, chỉ cần:

```text
value = (50/100) ​× 1023 = 511.5 ≈ 512
```

```cpp
analogWrite(ledPin, 512);
```

## PWM trong VDK ESP8266

- Có API chuẩn để hỗ trợ cho việc set giá trị Vref thông qua giá trị analog.

### `analogWrite`

- Hỗ trợ PWM để được giá trị Vref từ giá trị analog

- Cú pháp:

```cpp
analogWrite(pin, value);
```

- Với:

  - pin: là chân GPIO để điều khiển PWM

  - value: là giá trị analog tương ứng với Vref

### `analogWriteRange();`

- Là API dùng để - Thay đổi độ phân giải PWM:

Nếu bạn muốn thay đổi phạm vi PWM, dùng:

```cpp
analogWriteRange(new_range); // ví dụ: analogWriteRange(255);
```

### `analogWriteFreq`

- Là API dùng để thay đổi tần số => chu kỳ của xung

- Tần số PWM mặc định là khoảng 1 kHz, có thể thay đổi bằng:

```cpp
analogWriteFreq(frequency);
```

## 🔍 Một số lưu ý

- Sau khi gọi `analogWrite()`, chân GPIO sẽ phát tín hiệu PWM liên tục cho đến khi có lệnh mới.

- Nếu bạn gọi  `digitalWrite()` hoặc `digitalRead()` trên cùng chân, PWM sẽ bị dừng.
