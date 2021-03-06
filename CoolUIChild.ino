#include <SPI.h>
#include <AIR430BoostFCC.h>
#include <Servo.h>
#include <string.h>
#include "MspFlash.h"

/* child type as Node: VENT -> 0x60; FAN -> 0x70; BLIND -> 0x80 */
#define TYPE 0x60
#define SENSOR A4
#define SERVO  P2_0
#define RELAY  P2_2
#define STATUS P1_3
#define flash SEGMENT_D

Servo servo;

struct sPacket
{
  uint8_t upper, lower;
  uint8_t parent;
  uint8_t node;
  uint8_t msg[56];
};
struct sPacket rxPacket;
struct sPacket txPacket;
uint8_t ADDRESS_MASTER = 0x00;
uint8_t ADDRESS_PARENT = 0x99;
uint8_t ADDRESS_LOCAL = TYPE;
boolean initialized = false;
boolean _on = false;
unsigned char rom[] = {0x0A, 0x1A, 0x3A, 0x00};  // {Init ? 0x0B : 0x0A, ADDRESS_LOCAL, other}

void setup()
{
  Serial.begin(9600);
  pinMode(STATUS, OUTPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(STATUS, LOW);
  digitalWrite(RELAY, LOW);
  memset(txPacket.msg, 0, sizeof(txPacket.msg));
  txPacket.node = ADDRESS_LOCAL;
  Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
  servo.attach(SERVO);
  servo.write(0);
}

void loop()
{    
  // put your main code here, to run repeatedly:
  while (Radio.receiverOn((unsigned char*)&rxPacket, sizeof(rxPacket), 0) > 0) {
//    String message = (char*) rxPacket.msg;
    Serial.println((char*)rxPacket.msg);
    
    if (initialized && rxPacket.node == ADDRESS_LOCAL && (rxPacket.parent == ADDRESS_PARENT || rxPacket.parent == ADDRESS_MASTER)) {
//      Serial.println((char*)rxPacket.msg);
//      Serial.println(rxPacket.node);
//      Serial.println(message);
        if (!strcmp((char*)rxPacket.msg, "TEMP")) {
          getTemp();  // Using upper and lower in struct as data carrier
        } else if (!strcmp((char*)rxPacket.msg, "ON")) {
          if (!_on)
            on();
        } else if (!strcmp((char*)rxPacket.msg, "OFF")) {
          if (_on)
            off();
        } else if (!strcmp((char*)rxPacket.msg, "DEL")) {
          initialized = false;
          ADDRESS_LOCAL = TYPE;
          ADDRESS_PARENT = 0x99;
          txPacket.node = ADDRESS_LOCAL;
          while(Radio.busy()){}
          Radio.end();
          while(Radio.busy()){}
          Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
          while(Radio.busy()){}
          digitalWrite(STATUS, LOW);
          delay(500);
          digitalWrite(STATUS, HIGH);
          delay(500);
          digitalWrite(STATUS, LOW);
          delay(500);
          digitalWrite(STATUS, HIGH);
          delay(500);
          digitalWrite(STATUS, LOW);
        } else if (!strcmp((char*)rxPacket.msg, "STA")) {
          if (_on)
            txPacket.upper = 1;
          else
            txPacket.upper = 0;
        }
//        delay(1000);
        if (rxPacket.parent == ADDRESS_PARENT) Radio.transmit(ADDRESS_PARENT, (unsigned char*)&txPacket, sizeof(txPacket));
        else Radio.transmit(ADDRESS_MASTER, (unsigned char*)&txPacket, sizeof(txPacket));
    } else if (!initialized && !strcmp((char*)rxPacket.msg, "PAIR")) {
//      Serial.println(rxPacket.parent);
//      Serial.println(rxPacket.node);
//      Serial.println(message);
      ADDRESS_PARENT = rxPacket.parent;
      ADDRESS_LOCAL = rxPacket.node;
      txPacket.node = ADDRESS_LOCAL;
      strcpy((char*)txPacket.msg, "ACK");
      delay(500);
      Radio.transmit(ADDRESS_PARENT, (unsigned char*)&txPacket, sizeof(txPacket));
      digitalWrite(STATUS, HIGH);
      initialized = true;
      while(Radio.busy()){}
      Radio.end();
      while(Radio.busy()){}
      Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
    }
//    Serial.println("end");
  }
}

void getTemp() {
  int val = analogRead(SENSOR);
  val += analogRead(SENSOR);
  val = ceil(val / 2);
//  Serial.println(val);
  txPacket.upper = highByte(val);
  txPacket.lower = lowByte(val);
}

void on() {
  _on = true;
  if (TYPE == 0x70) {
    digitalWrite(RELAY, HIGH);
  } else {
    if (servo.attached()) {
//      for (int i = servo.read(); i <= 60; i++) {
//        servo.write(i);
//        delay(15);
//      }
      if (TYPE == 0x60)
        servo.write(60);
      else 
        servo.write(90);
    }
  }
}

void off() {
  _on = false;
  if (TYPE == 0x70) {
    digitalWrite(RELAY, LOW);
  } else {
    if (servo.attached()) {
//      for (int i = servo.read(); i >= 0; i--) {
//        servo.write(i);
//        delay(15); 
//      }
      servo.write(0);
    }
  }
}

unsigned char readFlash(uint8_t location) {
  unsigned char p = 0;
  int i=0;
  do {
    Flash.read(flash+i, &p, 1);
//    Serial.write(p);
//    Serial.print(":");    
//    Serial.println(p);
  } while (p && (i++ < location));
  return p;
}

void writeFlash() {
  Flash.erase(flash); 
  Flash.write(flash, rom, 4);
}
