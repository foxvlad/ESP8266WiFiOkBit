/*
  UDP OkBit protokol
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <OkbitUDP_ESP8266.h>
#include "DHT.h"

#define DHTPIN D1     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//define DHTTYPE DHT21   // DHT 21 (AM2301)


long last_mls = millis();

int time_period = 10000; //период опроса температуры и влажности

const char* ssid = "   ";
const char* password = "    ";

unsigned int localUdpPort = 6400;  // локальный порт для прослушки
char incomingPacket[100];          // буфер для входящих пакетов
char replyPacekt[100];  // буфер для исходящих пакетов

int sub_id = 0; //адресс подсети
int id = 0; //адрес устройства
int device = 7002; //тип устройства смотрим в модуле
int firmware[] = {0, 9}; //версия прошивки


WiFiUDP Udp;
OKBIT_UDP HandlerUDP(sub_id, id, device, firmware[0], firmware[1]);//подключение класса обработчика протокола OkBIT UDP
DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(115200);
  Serial.println();



  Serial.printf("Connecting to %s ", ssid); //  "Подключение к %s "
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected"); //  " подключено "

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort); //  "Теперь прослушиваем IP-адрес %s, UDP-порт %d"

  EEPROM.begin(512);
  HandlerUDP.eeprom_read();
  dht.begin();

}

void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // получаем входящие UDP-пакеты:
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort()); //  "Получено %d байт от %s, порт %d%"
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    HandlerUDP.parsing(incomingPacket, len, Udp.remoteIP());//обработка полученного пакета

    // отправляем ответ на IP-адрес и порт, с которых пришел пакет:
    if (HandlerUDP.status_err == 1) {
      HandlerUDP.status_err = 0;
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(HandlerUDP.replyGate);
      Udp.endPacket();
    }
  }

  HandlerUDP.holding_update();// Обработчик измениния регистров, в случае изменения состояния отправляеться запрос на сервер с передачей парамета
  if (HandlerUDP.status_err == 1) {
    HandlerUDP.status_err = 0;
    Serial.printf("UDP reply packet: %s\n", HandlerUDP.replyGate);
    IPAddress Gate(HandlerUDP.gateIP[0], HandlerUDP.gateIP[1], HandlerUDP.gateIP[2], HandlerUDP.gateIP[3]);
    Udp.beginPacket(Gate, 6500);
    Udp.write(HandlerUDP.replyGate);
    Udp.endPacket();
  }

  if (millis() - last_mls > time_period) {
    temperatura();
    last_mls = millis();
  }
}


void  temperatura(){

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  HandlerUDP.holdingRegs[1]  = t * 100;
  HandlerUDP.holdingRegs[2]  = h * 100;
}
