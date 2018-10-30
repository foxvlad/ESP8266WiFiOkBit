/*


*/

//******************************************************************************
//* Includes
//******************************************************************************

#include "Arduino.h"
#include "EEPROM.h"
#include "OkbitUDP_ESP8266.h"


//******************************************************************************
//* Support Functions
//******************************************************************************



//******************************************************************************
//* Constructors
//******************************************************************************


OKBIT_UDP::OKBIT_UDP(int sub_id, int id, int device, int firmware1, int firmware2) {
  _sub_id = sub_id;
  _id = id;
  _device = device;
  _firmware[0] = firmware1;
  _firmware[1] = firmware2;
}


//******************************************************************************
//* Public Methods
//******************************************************************************

void OKBIT_UDP::parsing(char inPacket[255], int len, uint32_t gateIP) {

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
  }

  unsigned int crcPack = ((packet_to_dec[(len / 2 - 1) - 1] << 8) | packet_to_dec[(len / 2 - 1)]);


  if (uStart != "4F4B4249542D554450AAAA") {

    this->status_err = 0;
  }
  else {
    this->status_err = 1;
    if (crc != crcPack) {
      this->status_err = 0;
    }
    else {
      this->status_err = 1;

      int long_pack  = packet_to_dec[11]; //длина пакета

      unsigned int in_sub_id = packet_to_dec[12]; //подсеть ID

      unsigned int in_id = packet_to_dec[13]; //ID

      unsigned int in_device = (packet_to_dec[14] << 8) | packet_to_dec[15] ; //код девайса

      in_cmd = (packet_to_dec[16] << 8) | packet_to_dec[17] ; //код команды

      unsigned int in_subto_id = packet_to_dec[18]; //подсеть ID

      unsigned int in_to_id = packet_to_dec[19]; //ID

      if (long_pack == 11 || long_pack == 13 || long_pack == 15 || long_pack == 17 || long_pack == 19 ) {
        _vol[1] = (packet_to_dec[20] << 8) | packet_to_dec[21] ; //значение 1
      }
      if (long_pack == 13 || long_pack == 15 || long_pack == 17 || long_pack == 19 ) {
        _vol[2] = (packet_to_dec[22] << 8) | packet_to_dec[23] ; //значение 2
      }
      if (long_pack == 15 || long_pack == 17 || long_pack == 19 ) {
        _vol[3] = (packet_to_dec[24] << 8) | packet_to_dec[25] ; //значение 2
      }
      if (long_pack == 17 || long_pack == 19 ) {
        _vol[4] = (packet_to_dec[26] << 8) | packet_to_dec[27] ; //значение 2
      }
      if (long_pack == 19 ) {
        _vol[5] = (packet_to_dec[28] << 8) | packet_to_dec[29] ; //значение 2
      }

    }
  }

  if (this->status_err == 1) {

    if (this->in_cmd == 30) {

      holdingRegs[_vol[1]] = _vol[2];
      _holdingRegs[_vol[1]] = _vol[2];
      String backup = "000B";
      backup.toCharArray(replyGate, backup.length() + 2);
    }


    if (this->in_cmd == 255 || this->in_cmd == 20) {
      unsigned long mid = ESP.getFlashChipId();
      unsigned int mid_b[2];
      mid_b[0] = mid >> 16;
      mid_b[1] = mid & 0xFFFF;

      this->build(_sub_id, _id, _device, 13, _sub_id, _id, _firmware[0], _firmware[1], mid_b[0], mid_b[1]);//передача верcие прошивки и серийного номера на сборку пакета
    }

    if (this->in_cmd == 64) {
      this->eeprom_write(gateIP);
      String backup = "000B";
      backup.toCharArray(replyGate, backup.length() + 2);
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

  b_pack.toCharArray(replyGate, b_pack.length() + 2);

}


void OKBIT_UDP::eeprom_read() {
  for (int i=0; i<4; i++) {
	gateIP[i] = EEPROM.read(i);
  }
  
  
}

void OKBIT_UDP::eeprom_write(uint32_t gateIP) {
  EEPROM.write(3, gateIP >> 24);
  EEPROM.write(2, (gateIP & 0xFF0000) >> 16);
  EEPROM.write(1, (gateIP & 0xFF00) >> 8);
  EEPROM.write(0, (gateIP & 0xFF));
  delay(100);
  EEPROM.commit();
  this->eeprom_read();
}



void OKBIT_UDP::holding_update() {
  for (int i = 0; i < 10; i++) {
    if (_holdingRegs[i] != holdingRegs[i]) { //если старое значение не равно новому
      _holdingRegs[i] = holdingRegs[i];//присвоить старому значению новое
      this->build(_sub_id, _id, _device, 30, _sub_id, _id, i, holdingRegs[i]);//передача верcие прошивки и серийного номера на сборку пакета
	  status_err = 1;
      return;
    }
    else status_err = 0;
    
  }
}