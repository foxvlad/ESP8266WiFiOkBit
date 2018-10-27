/*
  UDP OkBit protokol
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <Bounce.h>

const char* ssid = "OkBit";
const char* password = "wertualfox";

WiFiUDP Udp;


IPAddress MD(192, 168, 1, 35); //ip адрес MajorDoMo
unsigned int MD_Port = 6500;   //Порт MajorDoMo

unsigned int localUdpPort = 6400;  // локальный порт для прослушки
char incomingPacket[100];          // буфер для входящих пакетов
char replyPacekt[100];  // буфер для исходящих пакетов

int sub_id = 0; //адресс подсети
int id = 0; //адрес устройства
int device = 7001; //тип устройства
int firmware[] = {0, 9}; //версия прошивки


const int lamp1 = D6;
const int lamp2 = D7;

const int in1 = D1;
const int in2 = D2;

int holdingRegs[10];

class OKBIT_UDP {
  public:
    void parsing(char inPacket[255], int len); //Парсинг полученного пакета
    void build(int b_sub_id = 0, int b_id = 0, int b_device = 0, int b_cmd = 0 , int b_subto_id = 0, int b_to_id = 0, unsigned int b_vol1 = 0, unsigned int b_vol2 = 0, unsigned int b_vol3 = 0, unsigned int b_vol4 = 0); //Сборка пакета на отправку
    void send_event();
    void eeprom_read();
    void eeprom_write();
    void holding_update();
    int status_err;
    int in_cmd;
  private:
    int _vol[10]; // массив регистров управления и чтения данных
    int _holdingRegs[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};


OKBIT_UDP HandlerUDP;

// Instantiate a Bounce object with a 5 millisecond debounce time
Bounce bouncer1 = Bounce( in1, 10 );
Bounce bouncer2 = Bounce( in2, 10 );


void setup() {

  Serial.begin(115200);
  Serial.println();

  pinMode(lamp1, OUTPUT);
  pinMode(lamp2, OUTPUT);

  pinMode(in1, INPUT);
  pinMode(in2, INPUT);

  Serial.printf("Connecting to %s ", ssid);
  //  "Подключение к %s "
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  //  " подключено "

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  //  "Теперь прослушиваем IP-адрес %s, UDP-порт %d"

  EEPROM.begin(512);
}

void loop()
{
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // получаем входящие UDP-пакеты:
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    //  "Получено %d байт от %s, порт %d%"
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);


    HandlerUDP.parsing(incomingPacket, len);//разбор полученного пакета

    // отправляем ответ на IP-адрес и порт, с которых пришел пакет:
    if (HandlerUDP.status_err == 1) {
      Udp.beginPacket(MD, Udp.remotePort());
      Udp.write(replyPacekt);
      Udp.endPacket();
    }
  }

  digitalWrite(lamp1, holdingRegs[1]);
  digitalWrite(lamp2, holdingRegs[2]);
  debounce();
  HandlerUDP.holding_update();
}


void debounce() {

  if ( bouncer1.update() ) {
    if ( bouncer1.read() == LOW) {
      if ( holdingRegs[1] == LOW ) {
        holdingRegs[1] = HIGH;
      } else {
        holdingRegs[1] = LOW;
      }
    }
  }

  if ( bouncer2.update() ) {
    if ( bouncer2.read() == LOW) {
      if ( holdingRegs[2] == LOW ) {
        holdingRegs[2] = HIGH;
      } else {
        holdingRegs[2] = LOW;
      }
    }
  }
}
