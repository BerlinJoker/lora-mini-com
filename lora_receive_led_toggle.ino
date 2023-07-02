/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 22
#define dio0 21

int LED = 2;
int LEDtoggle = 0;

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;


void setup() {
  pinMode (LED, OUTPUT);
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");
  SerialBT.begin("ESP32loraRec"); //Bluetooth device name

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    SerialBT.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
      SerialBT.println(LoRaData); 
      if (LoRaData == "toggle"){
        if (LEDtoggle == 1) {
          digitalWrite(LED, LOW);
          SerialBT.println("led switched off"); 
          LEDtoggle = 0;
          SerialBT.println("led status set to 0"); 
        }
        else
        {
          digitalWrite(LED, HIGH);
          SerialBT.println("led switched on"); 
          LEDtoggle = 1;
          SerialBT.println("led status set to 1"); 
        }
      }
      
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    SerialBT.println("' RSSI:"); 
    Serial.println(LoRa.packetRssi());
    SerialBT.println(LoRa.packetRssi());

  }
}
