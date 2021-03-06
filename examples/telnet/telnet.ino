// Telnet越しにcliを動かすサンプル。
//
// WiFiライブラリのexample WiFiTelnetToSerial.inoを参考にしました。
//　
//   https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiTelnetToSerial/WiFiTelnetToSerial.ino
//

#include <Arduino.h>
#include <WiFi.h>
#include <cli.h>

#define MAX_SRV_CLIENTS 2

const char *ssid = "**********";
const char *password = "************";

static WiFiServer server(23);
static WiFiClient serverClients[MAX_SRV_CLIENTS];
static CLI cli[MAX_SRV_CLIENTS];

void setup(void) {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nReset.");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.printf("wifi connected IP:%s\n", WiFi.localIP().toString().c_str());
  server.begin();
  server.setNoDelay(true);

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    cli[i].cmd("hello", "hello command", [](CLI *cli, const char *args) {
      return cli->printf("Hello cmd:%s args:%s\n", cli->m_cmd, args);
    });
    cli[i].m_userData = i;
    cli[i].cmd("close", "close connection", [](CLI *cli, const char *args) {
      cli->printf("close connection\n");
      serverClients[cli->m_userData].stop();
      cli->init(NULL);
      return true;
    });
    cli[i].setEcho(false);
  }
}

void loop(void) {
  int i;
  if (WiFi.status() == WL_CONNECTED) {
    if (server.hasClient()) {
      for (i = 0; i < MAX_SRV_CLIENTS; i++) {
        if (!serverClients[i] || !serverClients[i].connected()) {
          if (serverClients[i]) serverClients[i].stop();
          serverClients[i] = server.available();
          if (!serverClients[i]) {
            Serial.println("available broken");
          } else {
            Serial.print("skip:");
            while (serverClients[i].available()) {
              uint8_t c = serverClients[i].read();
              Serial.printf(" %02x", c);
            }
            Serial.println();
            cli[i].init(&serverClients[i]);
            Serial.println("connected");
            cli[i].printf("\nConnected!!\n");
          }
        }
      }
      if (i >= MAX_SRV_CLIENTS) {
        server.available().stop();
      }
    }
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i]) {
        if (serverClients[i].connected()) {
          cli[i].update();
        } else {
          Serial.println("closed");
          serverClients[i].stop();
          cli[i].init(NULL);
        }
      }
    }
  } else {  // WiFi not concted
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
      if (serverClients[i]) {
        serverClients[i].stop();
      }
  }
}
