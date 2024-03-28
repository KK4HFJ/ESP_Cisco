# ESP_Cisco
 WiFi Remote Console Interface

Telnet into a ESP with the IP address (displayed on the LCD), and access a connected Cisco switch. Uses a MAX3232 TTL to RS232 converter.

Based on ESPTelnet library at https://github.com/LennartHennigs/ESPTelnet/

Still needs some work, as the Cisco prompts are doubled. Is completely functional.

ESP32 RX2 -> Max3232 TTL TX1

ESP32 TX2 -> Max3232 TTL RX1

Max3232 RS232 Out -> RJ45 pin 6

Max3232 RS232 In -> RJ45 pin 3

# Parts
RS232 to TTL - https://amzn.to/4azSUd5
I2C LCD - https://amzn.to/4a2Z8m5
ESP32 - https://amzn.to/3TUfxU7
Lithium Charger / 5v booster module - https://amzn.to/4ac3bwx
Lithium battery (6ah) - https://amzn.to/3TA5ger


