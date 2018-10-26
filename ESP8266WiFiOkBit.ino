#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
 
const char* ssid = "OkBit";
const char* password = "wertualfox";
 
WiFiUDP Udp;
unsigned int localUdpPort = 6400;  // локальный порт для прослушки
char incomingPacket[255];          // буфер для входящих пакетов
char  replyPacekt[] = "4F4B4249542D554450AAAA1100001B59000D000000000005071100060498";  // ответ
char  mas_on[] = "1";


 
void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(D8, OUTPUT);
 
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
              //  "Содержимое UDP-пакета: %s"
    if(incomingPacket[47] == '1'){
      Serial.println("ON");
       digitalWrite(D8, HIGH);
    }
    else {
      Serial.println("OFF");
      digitalWrite(D8, LOW);
    }
 
    // отправляем ответ на IP-адрес и порт, с которых пришел пакет:
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyPacekt);
    Udp.endPacket();
  }
}
