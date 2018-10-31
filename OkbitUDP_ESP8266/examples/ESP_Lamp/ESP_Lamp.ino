/*
  UDP OkBit protokol
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <Bounce.h>
#include <OkbitUDP_ESP8266.h>

const char* ssid = "   ";
const char* password = "  ";

unsigned int localUdpPort = 6400;  // локальный порт для прослушки
char incomingPacket[100];          // буфер для входящих пакетов
char replyPacekt[100];  // буфер для исходящих пакетов

int sub_id = 0; //адресс подсети
int id = 0; //адрес устройства
int device = 7001; //тип устройства смотрим в модуле
int firmware[] = {0,9}; //версия прошивки


const int lamp1 = D6; // выход управления лампой 1
const int lamp2 = D7; // выход управления лампой 2
const int lamp3 = D8; // выход управления лампой 3

const int in1 = D1; // вход управления лампой 1
const int in2 = D2; // вход управления лампой 2
const int in3 = D3; // вход управления лампой 3

Bounce bouncer1 = Bounce( in1, 10 ); // подавление дребезга контактов 1-го входа
Bounce bouncer2 = Bounce( in2, 10 ); // подавление дребезга контактов 2-го входа
Bounce bouncer3 = Bounce( in3, 10 ); // подавление дребезга контактов 3-го входа

WiFiUDP Udp;
OKBIT_UDP HandlerUDP(sub_id, id, device, firmware[0], firmware[1]);//подключение класса обработчика протокола OkBIT UDP

void setup() {

  Serial.begin(115200);
  Serial.println();

  pinMode(lamp1, OUTPUT); // выход управления лампой 1
  pinMode(lamp2, OUTPUT); // выход управления лампой 2
  pinMode(lamp3, OUTPUT); // выход управления лампой 3

  pinMode(in1, INPUT); // вход управления лампой 1
  pinMode(in2, INPUT); // вход управления лампой 2
  pinMode(in3, INPUT); // вход управления лампой 2

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

}

void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // получаем входящие UDP-пакеты:
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort()); //  "Получено %d байт от %s, порт %d%"
    int len = Udp.read(incomingPacket, 255);
    if (len > 0){
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

  debounce(); // вызов функции обработки нажатий кнопок

  digitalWrite(lamp1, HandlerUDP.holdingRegs[1]); // вкл\выкл - лампы в зависимости от пришедшей команды по UDP или по нажатию кнопки
  digitalWrite(lamp2, HandlerUDP.holdingRegs[2]);
  digitalWrite(lamp3, HandlerUDP.holdingRegs[3]); 
}


void debounce() { //Обработчик нажатия кнопок

  if ( bouncer1.update() ) {
    if ( bouncer1.read() == LOW) {
      if ( HandlerUDP.holdingRegs[1] == LOW ) {
        HandlerUDP.holdingRegs[1] = HIGH;
      } else {
        HandlerUDP.holdingRegs[1] = LOW;
      }
    }
  }

  if ( bouncer2.update() ) {
    if ( bouncer2.read() == LOW) {
      if ( HandlerUDP.holdingRegs[2] == LOW ) {
        HandlerUDP.holdingRegs[2] = HIGH;
      } else {
        HandlerUDP.holdingRegs[2] = LOW;
      }
    }
  }

  if ( bouncer3.update() ) {
    if ( bouncer3.read() == LOW) {
      if ( HandlerUDP.holdingRegs[3] == LOW ) {
        HandlerUDP.holdingRegs[3] = HIGH;
      } else {
        HandlerUDP.holdingRegs[3] = LOW;
      }
    }
  }
}
