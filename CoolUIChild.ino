#include <SPI.h>
#include <AIR430BoostFCC.h>
#include <Servo.h>
#include <string.h>

/* child type as Node: VENT -> 0x60; FAN -> 0x70; BLIND -> 0x80 */
#define TYPE 0x60
#define ADDRESS_PARENT 0x00  /* hardcoded parent node */
#define SENSOR A4
#define SERVO  P2_0
#define RELAY  P2_2
#define STATUS P1_3 

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
uint8_t ADDRESS_LOCAL = TYPE;
boolean initialized = false;

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
  while (Radio.receiverOn((unsigned char*)&rxPacket, sizeof(rxPacket), 0) > 0 && rxPacket.parent == ADDRESS_PARENT) {
//    String message = (char*) rxPacket.msg;
    Serial.println((char*)rxPacket.msg);
    
    if (initialized && rxPacket.node == ADDRESS_LOCAL) {
//      Serial.println((char*)rxPacket.msg);
//      Serial.println(rxPacket.node);
//      Serial.println(message);
        if (!strcmp((char*)rxPacket.msg, "TEMP")) {
          getTemp();  // Using upper and lower in struct as data carrier
        } else if (!strcmp((char*)rxPacket.msg, "ON")) {
          on();
        } else if (!strcmp((char*)rxPacket.msg, "OFF")) {
          off();
        } else if (!strcmp((char*)rxPacket.msg, "DEL")) {
          initialized = false;
          ADDRESS_LOCAL = TYPE;
          txPacket.node = ADDRESS_LOCAL;
          while(Radio.busy()){}
          Radio.end();
          while(Radio.busy()){}
          Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
          digitalWrite(STATUS, LOW);
          while(Radio.busy()){}
        }
//        delay(1000);
        Radio.transmit(ADDRESS_PARENT, (unsigned char*)&txPacket, sizeof(txPacket));
    } else if (!initialized && !strcmp((char*)rxPacket.msg, "PAIR")) {
//      Serial.println(rxPacket.parent);
//      Serial.println(rxPacket.node);
//      Serial.println(message);
      ADDRESS_LOCAL = rxPacket.node;
      strcpy((char*)txPacket.msg, "ACK");
      delay(1000);
      Radio.transmit(ADDRESS_PARENT, (unsigned char*)&txPacket, sizeof(txPacket));
      digitalWrite(STATUS, HIGH);
      initialized = true;
      while(Radio.busy()){}
      Radio.end();
      while(Radio.busy()){}
      Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
      txPacket.node = ADDRESS_LOCAL;
    } 
    
//    Serial.println("end");
  }
}

void getTemp() {
  int val = analogRead(SENSOR);
  val += analogRead(SENSOR);
  val = ceil(val / 2);
  txPacket.upper = highByte(val);
  txPacket.lower = lowByte(val);
}

void on() {
  if (TYPE == 0x70) {
    digitalWrite(RELAY, HIGH);
  } else {
    if (servo.attached()) {
      for (int i = servo.read(); i <= 90; i++) {
        servo.write(i);
        delay(15);
      }
    }
  }
}

void off() {
  if (TYPE == 0x70) {
    digitalWrite(RELAY, LOW);
  } else {
    if (servo.attached()) {
      for (int i = servo.read(); i >= 0; i--) {
        servo.write(i);
        delay(15); 
      }
    }
  }
}
