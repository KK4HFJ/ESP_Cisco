/*
  ESP_Cisco - WiFi telnet gateway to a Cisco console port
  -------------------------------------------------------
  Telnet to the ESP32 (its IP is shown on the LCD) and the session is bridged
  to the switch/router console through a MAX3232 on UART2.

  Wiring:
    ESP32 RX2 (GPIO16) <- MAX3232 TTL TX
    ESP32 TX2 (GPIO17) -> MAX3232 TTL RX
    MAX3232 RS232 out  -> RJ45 pin 6 (console RxD)
    MAX3232 RS232 in   <- RJ45 pin 3 (console TxD)
    I2C LCD 16x2       -> SDA GPIO21, SCL GPIO22

  Libraries: "ESP Telnet" (Lennart Hennigs), "LiquidCrystal I2C".
  WiFi credentials: copy secrets.example.h to secrets.h (git-ignored).
*/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPTelnet.h>
#include <LiquidCrystal_I2C.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Copy secrets.example.h to secrets.h and set your WiFi credentials."
#endif

// ---- Configuration ---------------------------------------------------------

constexpr uint32_t USB_SERIAL_BAUD = 9600;   // debug log on the USB port
constexpr uint32_t CONSOLE_BAUD    = 9600;   // Cisco console default, 8N1
constexpr uint16_t TELNET_PORT     = 23;

constexpr uint8_t  LCD_I2C_ADDR    = 0x27;
constexpr uint8_t  LCD_COLS        = 16;
constexpr uint8_t  LCD_ROWS        = 2;

constexpr int      WIFI_MAX_TRIES  = 20;
constexpr uint32_t WIFI_RETRY_MS   = 500;

// Max console bytes forwarded per loop() pass, so telnet.loop() keeps running
// even while the switch is dumping a long "show run".
constexpr size_t   CONSOLE_CHUNK   = 64;

// ---- Globals ---------------------------------------------------------------

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
ESPTelnet telnet;

// ---- Helpers ---------------------------------------------------------------

void lcdShow(const char *line0, const String &line1 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);
}

// Log the error, show it on the LCD, and reboot.
void fatal(const char *msg) {
  Serial.println(msg);
  Serial.println("Rebooting now...");
  lcdShow(msg, "Rebooting...");
  delay(2000);
  ESP.restart();
  for (;;) {}  // not reached
}

bool connectToWiFi(const char *ssid, const char *password) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);

  for (int i = 0; i < WIFI_MAX_TRIES && WiFi.status() != WL_CONNECTED; i++) {
    delay(WIFI_RETRY_MS);
    Serial.print('.');
  }
  Serial.println();

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  return WiFi.status() == WL_CONNECTED;
}

// ---- Telnet callbacks ------------------------------------------------------

void logTelnetEvent(const String &clientIp, const char *event) {
  Serial.print("- Telnet: ");
  Serial.print(clientIp);
  Serial.print(' ');
  Serial.println(event);
}

void onTelnetConnect(String clientIp) {
  logTelnetEvent(clientIp, "connected");
  telnet.println("\nWelcome " + telnet.getIP());
  telnet.println("(Use ^] + q  to disconnect.)");
}

void onTelnetDisconnect(String clientIp) {
  logTelnetEvent(clientIp, "disconnected");
}

void onTelnetReconnect(String clientIp) {
  logTelnetEvent(clientIp, "reconnected");
}

void onTelnetConnectionAttempt(String clientIp) {
  logTelnetEvent(clientIp, "tried to connect");
}

// Each line typed in the telnet session goes to the console, except two
// local commands handled by the gateway itself.
void onTelnetInput(String line) {
  if (line == "ping") {
    telnet.println("> pong");
    Serial.println("- Telnet: pong");
  } else if (line == "bye") {
    telnet.println("> disconnecting you...");
    telnet.disconnectClient();
  } else {
    Serial2.println(line);
  }
}

void setupTelnet() {
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);
  telnet.onInputReceived(onTelnetInput);

  if (!telnet.begin(TELNET_PORT)) {
    fatal("Telnet start failed");
  }
  Serial.println("- Telnet: running");
}

// ---- Arduino entry points --------------------------------------------------

void setup() {
  lcd.init();
  lcd.backlight();
  lcdShow("Getting IP ...");

  Serial.begin(USB_SERIAL_BAUD);
  delay(200);
  Serial.println();
  Serial.println("ESP_Cisco telnet gateway");

  Serial2.begin(CONSOLE_BAUD);

  Serial.print("- WiFi: ");
  if (!connectToWiFi(WIFI_SSID, WIFI_PASSWORD)) {
    fatal("WiFi connect failed");
  }

  const IPAddress ip = WiFi.localIP();
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.print(':');
  Serial.println(TELNET_PORT);

  // A full dotted quad is up to 15 chars, so it gets row 0 to itself.
  lcdShow(ip.toString().c_str(), "Telnet port " + String(TELNET_PORT));

  setupTelnet();
}

void loop() {
  telnet.loop();

  // Console output -> telnet client.
  for (size_t i = 0; i < CONSOLE_CHUNK && Serial2.available() > 0; i++) {
    telnet.write(Serial2.read());
  }
}
