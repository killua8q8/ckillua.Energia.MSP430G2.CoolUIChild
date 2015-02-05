#include <SPI.h>
#include <AIR430BoostFCC.h>
#include <string.h>

/* child type as Node: VENT -> 0x60; FAN -> 0x70; BLIND -> 0x80 */
#define TYPE 0x60
#define ADDRESS_PARENT 0x01

struct sPacket
{
  uint8_t parent;
  uint8_t node;
  uint8_t msg[58];
};
struct sPacket rxPacket;
struct sPacket txPacket;
uint8_t ADDRESS_LOCAL = TYPE;
boolean initialized = false;

void setup()
{
  Serial.begin(9600);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
//  firstInitializing();
  memset(txPacket.msg, 0, sizeof(txPacket.msg));
  txPacket.node = ADDRESS_LOCAL;
  Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
}

void loop()
{
  // put your main code here, to run repeatedly:
  while (Radio.receiverOn((unsigned char*)&rxPacket, sizeof(rxPacket), 0) > 0 && rxPacket.parent == ADDRESS_PARENT) {
//    String message = (char*) rxPacket.msg;
      Serial.println(ADDRESS_LOCAL);
    
    if (initialized && rxPacket.node == ADDRESS_LOCAL) {
//      Serial.println((char*)rxPacket.msg);
//      Serial.println(rxPacket.node);
//      Serial.println(message);
        if (!strcmp((char*)rxPacket.msg, "TEMP")) {
          txPacket.parent = getTemp();  // Using parent in struct as data carrier
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
          digitalWrite(RED_LED, LOW);
          while(Radio.busy()){}
        }
        delay(1000);
        Radio.transmit(ADDRESS_PARENT, (unsigned char*)&txPacket, sizeof(txPacket));
    } else if (!initialized && !strcmp((char*)rxPacket.msg, "PAIR")) {
//      Serial.println(rxPacket.parent);
//      Serial.println(rxPacket.node);
//      Serial.println(message);
      ADDRESS_LOCAL = rxPacket.node;
      strcpy((char*)txPacket.msg, "ACK");
      delay(1000);
      Radio.transmit(ADDRESS_PARENT, (unsigned char*)&txPacket, sizeof(txPacket));
      digitalWrite(RED_LED, HIGH);
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

uint8_t getTemp() {
  return 0;
}

void on() {
 
}

void off() {
  
}



















/************** OLD CODES **************
void firstInitializing() {
  Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
  txPacket.node = ADDRESS_LOCAL;
  memset(txPacket.msg, 0, sizeof(txPacket.msg));
  while(!firstInitialized) {
    while (Radio.receiverOn((unsigned char*)&rxPacket, sizeof(rxPacket), 0) <= 0) {}
    char* msg = (char*)rxPacket.msg;
    char smsg[4][10];
    uint8_t x = 0, y = 0;
    for (int i = 0; i < strlen(msg); i++) {
      if (msg[i] != ' ') {
        smsg[x][y] = msg[i];
        y++;
      } else {
        smsg[x][y] = '\0';
        x++;
        y=0;
      }
    }
    smsg[x][y] = '\0';
    ADDRESS_PARENT = rxPacket.node;
    ADDRESS_LOCAL = atoi(smsg[1]);
    txPacket.node = ADDRESS_LOCAL;
    delay(2000);
    Radio.transmit(ADDRESS_PAIR, (unsigned char*)&txPacket, sizeof(txPacket));
    Radio.setAddress(ADDRESS_LOCAL);
    delay(500);
    if (testConnection()) {
      firstInitialized = true;
      digitalWrite(RED_LED, HIGH);
    }
  }
  Serial.print("connection made to node: ");
  Serial.println(ADDRESS_PARENT);
}

boolean testConnection() {
//  Serial.println(ADDRESS_LOCAL);
  while (Radio.receiverOn((unsigned char*)&rxPacket, sizeof(rxPacket), 0) <= 0) {}
//  Serial.println(rxPacket.node);
  if (rxPacket.node = ADDRESS_PARENT) {
//     delay(2000);
    txPacket.node = 0x90;
    Radio.transmit(ADDRESS_PARENT, (unsigned char*)&txPacket, sizeof(txPacket));
    return true;
  }
  return false;
}
***************************************/
