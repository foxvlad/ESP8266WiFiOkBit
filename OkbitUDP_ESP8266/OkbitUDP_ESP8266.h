/*
  
*/

#ifndef OkbitUDP_ESP8266_h

#define OkbitUDP_ESP8266_h
#include "Arduino.h"
#include "EEPROM.h"


	

class OKBIT_UDP {
	
  public:
	OKBIT_UDP(int sub_id, int id, int device, int firmware1, int firmware2);
    void parsing(char inPacket[255], int len, uint32_t geteIP); //Парсинг полученного пакета
    void build(int b_sub_id = 0, int b_id = 0, int b_device = 0, int b_cmd = 0 , int b_subto_id = 0, int b_to_id = 0, unsigned int b_vol1 = 0, unsigned int b_vol2 = 0, unsigned int b_vol3 = 0, unsigned int b_vol4 = 0); //Сборка пакета на отправку
    //void send_event();
    void eeprom_read();
    void eeprom_write(uint32_t geteIP);
    void holding_update();
    int status_err;
    int in_cmd;
	int holdingRegs[10];
	char replyGate[100];
	int gateIP[4];
	
  private:
    int _vol[10]; // массив вспомогательный
    int _holdingRegs[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};// массив регистров для сравнения
	int _sub_id;
	int _id;
	int _device;
	int _firmware[2];
	char _replyPacekt[100];
};

//extern OKBIT_UDP HandlerUDP;

#endif 
