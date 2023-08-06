#include <SPI.h>
#include <LoRa.h>
#include "BluetoothSerial.h"

//LoRa pins
#define ss 5
#define rst 22
#define dio0 32


#define BUTTON1_PIN     27 // GIOP4 pin connected to button
#define BUTTON2_PIN     26 // GIOP4 pin connected to button
#define BUTTON3_PIN     25 // GIOP4 pin connected to button
#define BUTTON4_PIN     33 // GIOP4 pin connected to button
#define DEBOUNCE_TIME  50 // debounce time in milliseconds

// heartbeat helper variables, currently unused
int heartbeatmode = 0;
int needToSendHeartBeat = 0;

// minimum waiting times between heartbeat messages, currently unused
unsigned long heartbeatpause = 5000;
unsigned long heartbeatanyway = 10000;

// command timing definition and helper
unsigned long retrywaittime = 2000; // time to wait for command confirmation before a retry
unsigned long lasttry1 = 0;
unsigned long lasttry2 = 0;
unsigned long lasttry3 = 0;
unsigned long lasttry4 = 0;
int retrycount = 0;

// LED pin definitions and helpers
int LED = 2;
int LED2 = 4;
int LED3 = 16;
int LED4 = 17;
int checkLed1OffForRetry = 0;
int checkLed1OnForRetry = 0;
int checkLed2OffForRetry = 0;
int checkLed2OnForRetry = 0;
int checkLed3OffForRetry = 0;
int checkLed3OnForRetry = 0;
int checkLed4OffForRetry = 0;
int checkLed4OnForRetry = 0;

//toggle mode variables and helpers (test mode, currently not used)
int automode = 0; // flag if the sender currently runs in automode or not
unsigned long timebetweentoggle = 3000; // wait time between sending toggle messages
unsigned long lasttogglesent; // timestamp for the last actually sent toggle command

// button1 debounce variables and helpers
int lastSteadyState1 = LOW;       // the previous steady state from the input pin
int lastFlickerableState1 = LOW;  // the previous flickerable state from the input pin
int currentState1;                // the current reading from the input pin
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled

// button2 debounce variables and helpers
int lastSteadyState2 = LOW;       // the previous steady state from the input pin
int lastFlickerableState2 = LOW;  // the previous flickerable state from the input pin
int currentState2;                // the current reading from the input pin
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled

// button3 debounce variables and helpers
int lastSteadyState3 = LOW;       // the previous steady state from the input pin
int lastFlickerableState3 = LOW;  // the previous flickerable state from the input pin
int currentState3;                // the current reading from the input pin
unsigned long lastDebounceTime3 = 0;  // the last time the output pin was toggled

// button4 debounce variables and helpers
int lastSteadyState4 = LOW;       // the previous steady state from the input pin
int lastFlickerableState4 = LOW;  // the previous flickerable state from the input pin
int currentState4;                // the current reading from the input pin
unsigned long lastDebounceTime4 = 0;  // the last time the output pin was toggled


// bt serial setup
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;


void setup() {

  // LED setup
  pinMode (LED, OUTPUT);
  pinMode (LED2, OUTPUT);
  pinMode (LED3, OUTPUT);
  pinMode (LED4, OUTPUT);

  // Button setup
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);

  //serial outputs
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa node");
  
  SerialBT.begin("ESP32LoRaNodeB");  // bluetooth device name, change to different names for multiple nodes


  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  // initiate lora module
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  SerialBT.println("LoRa Initializing OK!'");

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

  // debounce logic for button 1
  // read the state of the switch/button:
  currentState1 = digitalRead(BUTTON1_PIN);
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
        // short blink for button press confirmation
        digitalWrite(LED, LOW);
        delay(100);
        digitalWrite(LED, HIGH);
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
        // short blink for button press confirmation
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
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


  // debounce logic for button 2
  // read the state of the switch/button:
  currentState2 = digitalRead(BUTTON2_PIN);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState2 != lastFlickerableState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
    // save the the last flickerable state
    lastFlickerableState2 = currentState2;
  }

  if ((millis() - lastDebounceTime2) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(lastSteadyState2 == HIGH && currentState2 == LOW){
      // button pressed down branch
      if(digitalRead(LED2)) // if LED2 is already on, pressing the button initiates the switch-off
      {
        // short blink for button press confirmation
        digitalWrite(LED2, LOW);
        delay(100);
        digitalWrite(LED2, HIGH);
        // tell other node to switch Led 2 off
        LoRa.beginPacket();
        LoRa.print("Led2off");
        LoRa.endPacket(); 
        lasttry2 = millis(); // remember when the message went out
        checkLed2OffForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }
      else
      {
        // short blink for button press confirmation
        digitalWrite(LED2, HIGH);
        delay(100);
        digitalWrite(LED2, LOW);
        // tell other node to switch Led 2 on
        LoRa.beginPacket();
        LoRa.print("Led2on");
        LoRa.endPacket(); 
        lasttry2 = millis(); // remember when the message went out
        checkLed2OnForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }

    }
    else if(lastSteadyState2 == LOW && currentState2 == HIGH)
    {
      // nothing happens here, this is the button release branch
    }

    // save the the last steady state
    lastSteadyState2 = currentState2;
  }


  // debounce logic for button 3
  // read the state of the switch/button:
  currentState3 = digitalRead(BUTTON3_PIN);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState3 != lastFlickerableState3) {
    // reset the debouncing timer
    lastDebounceTime3 = millis();
    // save the the last flickerable state
    lastFlickerableState3 = currentState3;
  }

  if ((millis() - lastDebounceTime3) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(lastSteadyState3 == HIGH && currentState3 == LOW){
      // button pressed down branch
      if(digitalRead(LED3)) // if LED3 is already on, pressing the button initiates the switch-off
      {
        // short blink for button press confirmation
        digitalWrite(LED3, LOW);
        delay(100);
        digitalWrite(LED3, HIGH);
        // tell other node to switch Led 3 off
        LoRa.beginPacket();
        LoRa.print("Led3off");
        LoRa.endPacket(); 
        lasttry3 = millis(); // remember when the message went out
        checkLed3OffForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }
      else
      {
        // short blink for button press confirmation
        digitalWrite(LED3, HIGH);
        delay(100);
        digitalWrite(LED3, LOW);
        // tell other node to switch Led 3 on
        LoRa.beginPacket();
        LoRa.print("Led3on");
        LoRa.endPacket(); 
        lasttry3 = millis(); // remember when the message went out
        checkLed3OnForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }

    }
    else if(lastSteadyState3 == LOW && currentState3 == HIGH)
    {
      // nothing happens here, this is the button release branch
    }

    // save the the last steady state
    lastSteadyState3 = currentState3;
  }


  // debounce logic for button 4
  // read the state of the switch/button:
  currentState4 = digitalRead(BUTTON4_PIN);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState4 != lastFlickerableState4) {
    // reset the debouncing timer
    lastDebounceTime4 = millis();
    // save the the last flickerable state
    lastFlickerableState4 = currentState4;
  }

  if ((millis() - lastDebounceTime4) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(lastSteadyState4 == HIGH && currentState4 == LOW){
      // button pressed down branch
      if(digitalRead(LED4)) // if LED4 is already on, pressing the button initiates the switch-off
      {
        // short blink for button press confirmation
        digitalWrite(LED4, LOW);
        delay(100);
        digitalWrite(LED4, HIGH);
        // tell other node to switch Led 4 off
        LoRa.beginPacket();
        LoRa.print("Led4off");
        LoRa.endPacket(); 
        lasttry4 = millis(); // remember when the message went out
        checkLed4OffForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }
      else
      {
        // short blink for button press confirmation
        digitalWrite(LED4, HIGH);
        delay(100);
        digitalWrite(LED4, LOW);
        // tell other node to switch Led 4 on
        LoRa.beginPacket();
        LoRa.print("Led4on");
        LoRa.endPacket(); 
        lasttry4 = millis(); // remember when the message went out
        checkLed4OnForRetry = 1; // activate check loop for retry
        // own LED only gets switched by confirmation command further down
      }

    }
    else if(lastSteadyState4 == LOW && currentState4 == HIGH)
    {
      // nothing happens here, this is the button release branch
    }

    // save the the last steady state
    lastSteadyState4 = currentState4;
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
   

// process incoming messages and set flags and lights accordingly
// check incoming messages

// try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    SerialBT.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.println(LoRaData); 
      SerialBT.println(LoRaData); 

      // checks for led1 commands

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

      // checks for led2 commands

      if (LoRaData == "Led2on") // command for led switch on received
      {
        digitalWrite(LED2, HIGH);
        LoRa.beginPacket();
        LoRa.print("Led2onConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led2off") // command for led switch off received
      {
        digitalWrite(LED2, LOW);
        LoRa.beginPacket();
        LoRa.print("Led2offConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led2onConfirm") // confirmation for led switch on received
      {
        digitalWrite(LED2, HIGH);
        checkLed2OnForRetry = 0; // no retry needed anymore
      }

      if (LoRaData == "Led2offConfirm") // confirmation for led switch off received
      {
        digitalWrite(LED2, LOW);
        checkLed2OffForRetry = 0; // no retry needed anymore
      } 


      // checks for led3 commands

      if (LoRaData == "Led3on") // command for led switch on received
      {
        digitalWrite(LED3, HIGH);
        LoRa.beginPacket();
        LoRa.print("Led3onConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led3off") // command for led switch off received
      {
        digitalWrite(LED3, LOW);
        LoRa.beginPacket();
        LoRa.print("Led3offConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led3onConfirm") // confirmation for led switch on received
      {
        digitalWrite(LED3, HIGH);
        checkLed3OnForRetry = 0; // no retry needed anymore
      }

      if (LoRaData == "Led3offConfirm") // confirmation for led switch off received
      {
        digitalWrite(LED3, LOW);
        checkLed3OffForRetry = 0; // no retry needed anymore
      } 

      // checks for led4 commands

      if (LoRaData == "Led4on") // command for led switch on received
      {
        digitalWrite(LED4, HIGH);
        LoRa.beginPacket();
        LoRa.print("Led4onConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led4off") // command for led switch off received
      {
        digitalWrite(LED4, LOW);
        LoRa.beginPacket();
        LoRa.print("Led4offConfirm");
        LoRa.endPacket(); 
      }

      if (LoRaData == "Led4onConfirm") // confirmation for led switch on received
      {
        digitalWrite(LED4, HIGH);
        checkLed4OnForRetry = 0; // no retry needed anymore
      }

      if (LoRaData == "Led4offConfirm") // confirmation for led switch off received
      {
        digitalWrite(LED4, LOW);
        checkLed4OffForRetry = 0; // no retry needed anymore
      } 





         
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
  // button/led 1 retries
  if ((checkLed1OffForRetry == 1) && (millis() > (lasttry1 + retrywaittime))) // if no confirmation for led1 off command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    // tell other node to switch Led 1 off, retry
    LoRa.beginPacket();
    LoRa.print("Led1off");
    LoRa.endPacket(); 
    lasttry1 = millis(); // remember when the retry went out
    retrycount++;
  }
  
  if ((checkLed1OnForRetry == 1) && (millis() > (lasttry1 + retrywaittime))) // if no confirmation for led1 on command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    // tell other node to switch Led 1 on, retry
    LoRa.beginPacket();
    LoRa.print("Led1on");
    LoRa.endPacket(); 
    lasttry1 = millis(); // remember when the retry went out
    retrycount++;
  }

  // button/led 2 retries
  if ((checkLed2OffForRetry == 1) && (millis() > (lasttry2 + retrywaittime))) // if no confirmation for led2 off command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED2, LOW);
    delay(100);
    digitalWrite(LED2, HIGH);
    // tell other node to switch Led 2 off (retry)
    LoRa.beginPacket();
    LoRa.print("Led2off");
    LoRa.endPacket(); 
    lasttry2 = millis(); // remember when the retry went out
    retrycount++;
  }
  
  if ((checkLed2OnForRetry == 1) && (millis() > (lasttry2 + retrywaittime))) // if no confirmation for led2 on command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED2, HIGH);
    delay(100);
    digitalWrite(LED2, LOW);
    // tell other node to switch Led 2 on (retry)
    LoRa.beginPacket();
    LoRa.print("Led2on");
    LoRa.endPacket(); 
    lasttry2 = millis(); // remember when the retry went out
    retrycount++;
  }


  // button/led 3 retries
  if ((checkLed3OffForRetry == 1) && (millis() > (lasttry3 + retrywaittime))) // if no confirmation for led2 off command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED3, LOW);
    delay(100);
    digitalWrite(LED3, HIGH);
    // tell other node to switch Led 2 off (retry)
    LoRa.beginPacket();
    LoRa.print("Led3off");
    LoRa.endPacket(); 
    lasttry3 = millis(); // remember when the retry went out
    retrycount++;
  }
  
  if ((checkLed3OnForRetry == 1) && (millis() > (lasttry3 + retrywaittime))) // if no confirmation for led3 on command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED3, HIGH);
    delay(100);
    digitalWrite(LED3, LOW);
    // tell other node to switch Led 3 on (retry)
    LoRa.beginPacket();
    LoRa.print("Led3on");
    LoRa.endPacket(); 
    lasttry3 = millis(); // remember when the retry went out
    retrycount++;
  }

  // button/led 4 retries
  if ((checkLed4OffForRetry == 1) && (millis() > (lasttry4 + retrywaittime))) // if no confirmation for led4 off command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED4, LOW);
    delay(100);
    digitalWrite(LED4, HIGH);
    // tell other node to switch Led 4 off (retry)
    LoRa.beginPacket();
    LoRa.print("Led4off");
    LoRa.endPacket(); 
    lasttry4 = millis(); // remember when the retry went out
    retrycount++;
  }
  
  if ((checkLed4OnForRetry == 1) && (millis() > (lasttry4 + retrywaittime))) // if no confirmation for led4 on command received yet, and retry waittime reached
  {
    // short blink for retry info
    digitalWrite(LED4, HIGH);
    delay(100);
    digitalWrite(LED4, LOW);
    // tell other node to switch Led 4 on (retry)
    LoRa.beginPacket();
    LoRa.print("Led4on");
    LoRa.endPacket(); 
    lasttry4 = millis(); // remember when the retry went out
    retrycount++;
  }

}
