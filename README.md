# ESP_Cisco (WiFi Telnet Gateway)

WiFi remote console interface for Cisco switches and routers.

Telnet to the ESP32 at the IP address shown on the LCD, and you're on the console of the attached Cisco device. A MAX3232 converts the ESP32's 3.3 V UART to RS-232 levels.

Built on the [ESPTelnet](https://github.com/LennartHennigs/ESPTelnet/) library. The sketch is fully working.

## Wiring

| From | To |
|---|---|
| ESP32 RX2 (GPIO16) | MAX3232 TTL TX1 |
| ESP32 TX2 (GPIO17) | MAX3232 TTL RX1 |
| MAX3232 RS232 out | RJ45 pin 6 (console RxD) |
| MAX3232 RS232 in | RJ45 pin 3 (console TxD) |
| I2C LCD SDA / SCL | GPIO21 / GPIO22 |

See `MAX3232.png` and `RJ45.png`.

## Setup

1. Install the ESP32 board core and these libraries from the Library Manager: **ESP Telnet** (Lennart Hennigs) and **LiquidCrystal I2C**.
2. Copy `esp32_telnet/secrets.example.h` to `esp32_telnet/secrets.h` and enter your WiFi SSID and password. `secrets.h` is git-ignored.
3. Open `esp32_telnet/esp32_telnet.ino`, select your ESP32 board, and upload.
4. `telnet <ip shown on LCD>` (port 23). The console runs at 9600 8N1.

Typed lines are sent to the console when you press Enter. Two words are handled by the gateway instead of the switch: `ping` replies `pong`, and `bye` disconnects you.

## Parts

- RS232 to TTL - https://amzn.to/4azSUd5
- I2C LCD - https://amzn.to/4a2Z8m5
- ESP32 - https://amzn.to/3TUfxU7
- Lithium charger / 5 V booster module - https://amzn.to/4ac3bwx
- Lithium battery (6 Ah) - https://amzn.to/3TA5ger
