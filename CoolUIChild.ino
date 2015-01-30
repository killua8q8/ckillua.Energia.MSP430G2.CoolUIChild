#include <SPI.h>
#include <AIR430BoostFCC.h>
#include <string.h>

#define TYPE "fan"

struct sPacket
{
  uint8_t node;
  uint8_t msg[59];
};
struct sPacket rxPacket;
struct sPacket txPacket;
uint8_t ADDRESS_PAIR = 0x00;    // using 0x00 when pairing
uint8_t ADDRESS_PARENT = 0x00;  // assigned when paried
uint8_t ADDRESS_LOCAL = 0x5F;   // using 0x5F as base when paring, changes when paried with assigned value
boolean firstInitialized = false;

void setup()
{
  Serial.begin(9600);
  firstInitializing();
}

void loop()
{
  // put your main code here, to run repeatedly:
  
}

void firstInitializing() {
  Radio.begin(ADDRESS_LOCAL, CHANNEL_1, POWER_MAX);
  txPacket.node = ADDRESS_LOCAL;
  memset(txPacket.msg, 0, sizeof(txPacket.msg));
  while(!firstInitialized) {
    while (Radio.receiverOn((unsigned char*)&rxPacket, sizeof(rxPacket), 0) <= 0) {}
    char* msg = (char*)rxPacket.msg;
    char smsg[6][10];
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
    ADDRESS_LOCAL = atoi(smsg[3]);
    txPacket.node = ADDRESS_LOCAL;
    delay(500);
    Radio.transmit(ADDRESS_PAIR, (unsigned char*)&txPacket, sizeof(txPacket));
    firstInitialized = true;    
  }
  Radio.setAddress(ADDRESS_LOCAL);
  Serial.print("connection made to node: ");
  Serial.println(ADDRESS_PARENT);
}

uint8_t getTemp() {
  return 0;
}
