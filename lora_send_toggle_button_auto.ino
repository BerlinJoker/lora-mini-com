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

#define BUTTON_PIN     4 // GIOP4 pin connected to button
#define DEBOUNCE_TIME  50 // the debounce time in millisecond, increase this time if it still chatters

int LED = 2;
int LEDtoggle = 0;

int counter = 0;

int automode = 0; // flag if the sender currently runs in automode or not

unsigned long timebetweentoggle = 3000; // wait time between sending toggle messages
unsigned long lasttogglesent; // timestamp for the last actually sent toggle command

int lastSteadyState = LOW;       // the previous steady state from the input pin
int lastFlickerableState = LOW;  // the previous flickerable state from the input pin
int currentState;                // the current reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled


#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
  
  // set mode for onboard LED
  pinMode (LED, OUTPUT);
  
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");
    SerialBT.begin("ESP32loraSend"); //Bluetooth device name

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

  // initialise the button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {

  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState != lastFlickerableState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    // save the the last flickerable state
    lastFlickerableState = currentState;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(lastSteadyState == HIGH && currentState == LOW){
       if (automode == 0)
       {
        automode = 1;
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
       }
       else
       {
        automode = 0;
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
       }
    }
    else if(lastSteadyState == LOW && currentState == HIGH)
    {
      Serial.println("The button is released");
    }

    // save the the last steady state
    lastSteadyState = currentState;
  }

  if ((millis() > (lasttogglesent + timebetweentoggle)) && automode == 1 ) // if time between toggles was reached, and board is in automode
  { 
      //Send LoRa packet to receiver
      LoRa.beginPacket();
      LoRa.print("toggle");
      LoRa.endPacket(); 

      // reset timer after toggle sent
      lasttogglesent = millis();

      // short blink for confirmation
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);

      // logging for sent command
      SerialBT.println("toggle command sent"); 
      Serial.println("toggle command sent"); 
  }

  

}
