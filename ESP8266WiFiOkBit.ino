/*
  UDP OkBit protokol


*/


#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "OkBit";
const char* password = "wertualfox";

WiFiUDP Udp;
unsigned int localUdpPort = 6400;  // локальный порт для прослушки
char incomingPacket[100];          // буфер для входящих пакетов
char  replyPacekt[100];  // ответ



int sub_id = 0; //адресс подсети
int id = 0; //адрес устройства
int device = 7001; //тип устройства
int firmware[] = 0,9;



int vol[10]; // массив регистров управления и чтения данных

const int lamp1 = D6;
const int lamp2 = D7;


class OKBIT_UDP {
  public:
    void parsing(char inPacket[255], int len); //Парсинг полученного пакета
    void build(int b_sub_id = 0, int b_id = 0, int b_device = 0, int b_cmd = 0 , int b_subto_id = 0, int b_to_id = 0, unsigned int b_vol1 = 0, unsigned int b_vol2 = 0, unsigned int b_vol3 = 0, unsigned int b_vol4 = 0); //Сборка пакета на отправку
    int in_cmd;
    int status_err;
  private:

};

void OKBIT_UDP::parsing(char inPacket[255], int len) {

  String uStart;//udp start
  int packet_to_dec[30];// массив значений пакета в DEC
  char bufChar[2]; // буферная вспомогательная строка
  unsigned int crc; // контрольная сумма пакета

  for (int i = 0; i < 22; i = i + 1) { //udp start packet
    uStart = uStart + String(inPacket[i]);
  }

  crc = 0;
  for (int i = 0; i < len; i = i + 2) { //разбивка пакета на байты
    (String(inPacket[i]) + String(inPacket[1 + i])).toCharArray(bufChar, 3);
    sscanf(bufChar, "%x", &packet_to_dec[i / 2]);
    if (len - 4 > i) crc = crc + packet_to_dec[i / 2];
    Serial.print(bufChar);
    Serial.print(" | ");
  }

  unsigned int crcPack = ((packet_to_dec[(len / 2 - 1) - 1] << 8) | packet_to_dec[(len / 2 - 1)]);

  Serial.println("");
  if (uStart != "4F4B4249542D554450AAAA") {
    Serial.println("UDP Start ERROR");
    status_err = 0;
  }
  else {
    Serial.println("UDP Start it is OK");
    status_err = 1;
    if (crc != crcPack) {
      Serial.println("crc it is ERROR");
      status_err = 0;
    }
    else {
      Serial.println("crc it is OK");

      status_err = 1;

      int long_pack  = packet_to_dec[11]; //длина пакета
      Serial.print("Length - ");
      Serial.println(long_pack);

      unsigned int in_sub_id = packet_to_dec[12]; //подсеть ID
      Serial.print("Sub_ID - ");
      Serial.println(in_sub_id);

      unsigned int in_id = packet_to_dec[13]; //ID
      Serial.print("ID - ");
      Serial.println(in_id);

      unsigned int in_device = (packet_to_dec[14] << 8) | packet_to_dec[15] ; //код девайса
      Serial.print("Device - ");
      Serial.println(in_device);

      in_cmd = (packet_to_dec[16] << 8) | packet_to_dec[17] ; //код команды
      Serial.print("CMD - ");
      Serial.println(in_cmd);

      unsigned int in_subto_id = packet_to_dec[18]; //подсеть ID
      Serial.print("Subto_ID - ");
      Serial.println(in_subto_id);

      unsigned int in_to_id = packet_to_dec[19]; //ID
      Serial.print("to_ID - ");
      Serial.println(in_to_id);

      if (long_pack == 11 || long_pack == 13 || long_pack == 15 || long_pack == 17 || long_pack == 19 ) {
        vol[1] = (packet_to_dec[20] << 8) | packet_to_dec[21] ; //значение 1
        Serial.print("vol[1] - ");
        Serial.println(vol[1]);
      }
      if (long_pack == 13 || long_pack == 15 || long_pack == 17 || long_pack == 19 ) {
        vol[2] = (packet_to_dec[22] << 8) | packet_to_dec[23] ; //значение 2
        Serial.print("vol[2] - ");
        Serial.println(vol[2]);
      }
      if (long_pack == 15 || long_pack == 17 || long_pack == 19 ) {
        vol[3] = (packet_to_dec[24] << 8) | packet_to_dec[25] ; //значение 2
        Serial.print("vol[3] - ");
        Serial.println(vol[3]);
      }

      if (long_pack == 17 || long_pack == 19 ) {
        vol[4] = (packet_to_dec[26] << 8) | packet_to_dec[27] ; //значение 2
        Serial.print("vol[4] - ");
        Serial.println(vol[4]);
      }
      if (long_pack == 19 ) {
        vol[5] = (packet_to_dec[28] << 8) | packet_to_dec[29] ; //значение 2
        Serial.print("vol[5] - ");
        Serial.println(vol[5]);
      }

    }
  }



}


void OKBIT_UDP::build(int b_sub_id, int b_id, int b_device, int b_cmd, int b_subto_id, int b_to_id, unsigned int b_vol1, unsigned int b_vol2, unsigned int b_vol3, unsigned int b_vol4) {
  String b_pack;
  char myStr[3];
  int b_len_pack;
  String buf_pack;

  b_pack = "4F4B4249542D554450AAAA";

  if (b_cmd == 30) b_len_pack = 13;
  if (b_cmd == 13) b_len_pack = 17;

  sprintf(myStr, "%02X", b_len_pack );
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_sub_id );
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_id );
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_device >> 8);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_device & 0xFF);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_cmd >> 8);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_cmd & 0xFF);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_subto_id);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", b_to_id);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  if (b_len_pack == 13 || b_len_pack == 15 || b_len_pack == 17) {
    sprintf(myStr, "%02X", b_vol1 >> 8);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;

    sprintf(myStr, "%02X", b_vol1 & 0xFF);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;

    sprintf(myStr, "%02X", b_vol2 >> 8);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;

    sprintf(myStr, "%02X", b_vol2 & 0xFF);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;
  }

  if (b_len_pack == 15 || b_len_pack == 17) {
    sprintf(myStr, "%02X", b_vol3 >> 8);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;

    sprintf(myStr, "%02X", b_vol3 & 0xFF);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;
  }

  if ( b_len_pack == 17) {
    sprintf(myStr, "%02X", b_vol4 >> 8);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;

    sprintf(myStr, "%02X", b_vol4 & 0xFF);
    buf_pack = myStr;
    b_pack = b_pack +  buf_pack;
  }



  int packet_to_dec[30];
  char b_myStr[100];
  char bufChar[2];
  int len = b_pack.length() + 3;

  b_pack.toCharArray( b_myStr, len);



  unsigned int crc = 0;
  for (int i = 0; i < len; i = i + 2) { //разбивка пакета на байты
    (String( b_myStr[i]) + String( b_myStr[1 + i])).toCharArray(bufChar, 3);
    sscanf(bufChar, "%x", &packet_to_dec[i / 2]);
    if (len - 4 > i) crc = crc + packet_to_dec[i / 2];
  }

  sprintf(myStr, "%02X", crc >> 8);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;

  sprintf(myStr, "%02X", crc & 0xFF);
  buf_pack = myStr;
  b_pack = b_pack +  buf_pack;


  b_pack.toCharArray(replyPacekt, b_pack.length()+2);

  Serial.println("");
  Serial.print("HEX - ");
  Serial.println(replyPacekt);


}










void setup()
{



  Serial.begin(115200);
  Serial.println();

  pinMode(lamp1, OUTPUT);
  pinMode(lamp2, OUTPUT);

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

    OKBIT_UDP DownPacket;
    DownPacket.parsing(incomingPacket, len);

    int lamp;

    if ( DownPacket.status_err == 1 && DownPacket.in_cmd == 30 ) {
      if (vol[1] == 1) lamp = lamp1;
      if (vol[1] == 2) lamp = lamp2;
      digitalWrite(lamp, vol[2]);
      String backup = "000B";
      backup.toCharArray(replyPacekt, backup.length()+2);
    }


    if ( DownPacket.status_err == 1 && DownPacket.in_cmd == 255) {
      unsigned long mid = ESP.getFlashChipId();
      unsigned int mid_b[2];
      mid_b[0] = mid >> 16;
      mid_b[1] = mid & 0xFFFF;

      DownPacket.build(sub_id, id, device, 13, sub_id, id, firmware[0], firmware[1], mid_b[0], mid_b[1]);//передача верчие прошивки и серийного номера
    }

    // отправляем ответ на IP-адрес и порт, с которых пришел пакет:
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyPacekt);
    Udp.endPacket();
  }

}
