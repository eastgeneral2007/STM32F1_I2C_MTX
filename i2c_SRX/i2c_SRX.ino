#include <Wire.h>
#define OWN_ADDRESS  0x29
#define LED_PIN      13
int ledBlink = 0;
uint8_t receivedByte;

void receiveEvent(int bytes){
    receivedByte = Wire.read();
}

void setup(){
    Serial.begin(9600);
    pinMode (LED_PIN, OUTPUT);
    Wire.begin(OWN_ADDRESS);
    Wire.onReceive(receiveEvent);
}

void loop(){
  Serial.println(receivedByte);
    if (receivedByte == 0xF7){
        digitalWrite(LED_PIN, HIGH);
    }
    else{
        digitalWrite(LED_PIN, LOW);
    }
}

