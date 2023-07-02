

heartbeatmode = 0
needToSendHeartBeat = 0

heartbeatpause = 5000
heartbeatanyway = 10000

debouncetime = 50

retrywaittime = 2000 // time to wait for command confirmation before a retry
lasttry =


void setup() {
  // put your setup code here, to run once:

  leds
  buttons
  lora
  serial
  serialbt

}

void loop() {
  // put your main code here, to run repeatedly:


 check button 1 (if pressed down)
   if led 1 = on
    send "switch off 1"
    lasttry = millis
    checkXOffforretry = 1
    switch led 1 off // to remove when switching off is done via roundtrip confirmation message
   else
    send "switch on 1"
    lasttry = millis
    checkXOnforretry = 1
    switch led 1 on // to remove when switching on is done via roundtrip confirmation message



 check button heartbeat
   if heartbeat mode = 0
     heartbeatmode = 1
     needtosendheartbeat = 1
   else
     send "heartbeat off"
     switch off heartbeat led
     heartbeatmode = 0
   

 // processing incoming messages and set flags and lights accordingly
 check incoming message

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

   
// heartbeat processing
 if (heartbeatmode = 1 and needToSendHeartBeat = 1 and heartbeatpause long enough  (5 seconds)) OR (heartbeatmode = 1 and sendAnywayTime long enough (10 seconds?))
     send heartbeat
     needToSendHeartBeat = 0
     lastHeartBeatSend = millis

// generic message sending if needed
if messagetosend = 1
     send message x
     messagetosend = 0

// retry per command, should be generalised (one function for all retry scenarios)
if checkxoffforretry = 1 and millis > lasttry + retrywaittime
     send switch off X
     lasttry = millis

if checkxonforretry = 1 and millis > lasttry + retrywaittime
    send switch on X
    lasttry = millis

}
