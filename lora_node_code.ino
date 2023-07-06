#include <SPI.h>
#include <LoRa.h>
#include "BluetoothSerial.h"

//LoRa pins
#define ss 5
#define rst 22
#define dio0 21


#define BUTTON_PIN     4 // GIOP4 pin connected to button
#define DEBOUNCE_TIME  50 // debounce time in milliseconds

// heartbeat helper variables
int heartbeatmode = 0;
int needToSendHeartBeat = 0;

// minimum waiting times between heartbeat messages
unsigned long heartbeatpause = 5000;
unsigned long heartbeatanyway = 10000;

// command timing definition and helper
unsigned long retrywaittime = 2000; // time to wait for command confirmation before a retry
unsigned long lasttry1 = 0;
int retrycount = 0;

// LED pin definitions and helpers
int LED = 2;
int LEDstatus = 0;
int checkLed1OffForRetry = 0;
int checkLed1OnForRetry = 0;

//toggle mode variables and helpers (test mode)
int automode = 0; // flag if the sender currently runs in automode or not
unsigned long timebetweentoggle = 3000; // wait time between sending toggle messages
unsigned long lasttogglesent; // timestamp for the last actually sent toggle command

// button debounce variables and helpers
int lastSteadyState1 = LOW;       // the previous steady state from the input pin
int lastFlickerableState1 = LOW;  // the previous flickerable state from the input pin
int currentState1;                // the current reading from the input pin
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled

// bt serial setup
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;


void setup() {

  // LED setup
  pinMode (LED, OUTPUT);

  // Button setup
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //serial outputs
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa node");
  SerialBT.begin("ESP32LoRaNode"); 


  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  // initiate lora module
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  SerialBT.print("LoRa Initializing OK!'");

  //visual signal for working LoRa setup
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

  // debounce logic per button
  // read the state of the switch/button:
  currentState1 = digitalRead(BUTTON_PIN);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState1 != lastFlickerableState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();
    // save the the last flickerable state
    lastFlickerableState1 = currentState1;
  }

  if ((millis() - lastDebounceTime1) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(lastSteadyState1 == HIGH && currentState1 == LOW){
      // button pressed down branch
      if(digitalRead(LED)) // if LED1 is already on, pressing the button initiates the switch-off
      {
        // tell other node to switch Led 1 off
        LoRa.beginPacket();
        LoRa.print("Led1off");
        LoRa.endPacket(); 
        lasttry1 = millis(); // remember when the message went out
        checkLed1OffForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }
      else
      {
        // tell other node to switch Led 1 on
        LoRa.beginPacket();
        LoRa.print("Led1on");
        LoRa.endPacket(); 
        lasttry1 = millis(); // remember when the message went out
        checkLed1OnForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }

    }
    else if(lastSteadyState1 == LOW && currentState1 == HIGH)
    {
      // nothing happens here, this is the button release branch
    }

    // save the the last steady state
    lastSteadyState1 = currentState1;
  }



/*

// pseudo code for heartbeat button
 check button heartbeat
   if heartbeat mode = 0
     heartbeatmode = 1
     needtosendheartbeat = 1
   else
     send "heartbeat off"
     switch off heartbeat led
     heartbeatmode = 0

*/
   

// processing incoming messages and set flags and lights accordingly
// check incoming message

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

      if (LoRaData == "Led1on") // command for led switch on received
      {
        digitalWrite(LED, HIGH);
        LoRa.beginPacket();
        LoRa.print("Led1onConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led1off") // command for led switch off received
      {
        digitalWrite(LED, LOW);
        LoRa.beginPacket();
        LoRa.print("Led1offConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led1onConfirm") // confirmation for led switch on received
      {
        digitalWrite(LED, HIGH);
        checkLed1OnForRetry = 0; // no retry needed anymore
      }

      if (LoRaData == "Led1offConfirm") // confirmation for led switch off received
      {
        digitalWrite(LED, LOW);
        checkLed1OffForRetry = 0; // no retry needed anymore
      }

      /*
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
      */
      
    }
  }

/*
   if "heartbeat" command
   heartbeatmode = 1
   needToSendHeartBeat = 1
   switch on heartbeat led
   delay 500
   switch off heartbeat led

   if "heartbeat off" command
   heartbeatmode = 0
   needToSendHeartBeat = 0
   switch off heartbeat led // just in case, alternativly: 3 short blinks
*/

/*
   if "switch on x" command 
     switch on led x
     send "led x on confirmed" // only for roundtrip usage

   if "switch off x" command
     switch off led x
     send "led x off confirmed" // only for roundtrip usage

   if "led x off confirmed" // own led only switched off after confirmed roundtrip
     switch led x off
     checkXOffforretry = 0

   if "led x on confirmed" // own led only switched off after confirmed roundtrip
     switch led x on
     checkXOnforretry = 0
*/

/*
// heartbeat processing
 if (heartbeatmode = 1 and needToSendHeartBeat = 1 and heartbeatpause long enough  (5 seconds)) OR (heartbeatmode = 1 and sendAnywayTime long enough (10 seconds?))
     send heartbeat
     needToSendHeartBeat = 0
     lastHeartBeatSend = millis

// generic message sending if needed
if messagetosend = 1
     send message x
     messagetosend = 0
*/

  // retry per command, should be generalised (one function for all retry scenarios)
  if ((checkLed1OffForRetry == 1) && (millis() > (lasttry1 + retrywaittime)))
  {
    // tell other node to switch Led 1 off, retry
    LoRa.beginPacket();
    LoRa.print("Led1off");
    LoRa.endPacket(); 
    lasttry1 = millis(); // remember when the retry went out
    retrycount++;
  }
  
  if ((checkLed1OnForRetry == 1) && (millis() > (lasttry1 + retrywaittime)))
  {
    // tell other node to switch Led 1 on, retry
    LoRa.beginPacket();
    LoRa.print("Led1on");
    LoRa.endPacket(); 
    lasttry1 = millis(); // remember when the retry went out
    retrycount++;
  }


}
