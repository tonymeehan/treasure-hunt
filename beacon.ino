#include <ArduinoBLE.h>
#include "pitches.h"

// https://github.com/porrey/max1704x
#include "MAX17043.h"

#define LED_PIN 11
#define MOSFET_PIN 7
#define SPEAKER_PIN 8

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

BLEService beaconService("8090d5bb-59b8-40ca-acb9-443fdc12c4f2");
BLEUnsignedCharCharacteristic chirp("cba93d37-cae3-44db-88cb-b38c3613ee54", BLERead | BLEWriteWithoutResponse | BLENotify); 
BLEUnsignedCharCharacteristic clue("339a1fed-3da8-414b-8a61-67afc4b8c260", BLERead | BLENotify);
BLEStringCharacteristic battery("762e6190-0210-42b2-97c3-a699afcf1287", BLERead | BLENotify, 32);

byte doEcho = 0;  
long previousMillis = 0;
float batteryPercent = 0.0;

void setup() {
  Serial.begin(9600);
  FuelGauge.begin();
  
  if (!BLE.begin()) {
    while (1);
  }

  BLE.setLocalName("Beacon");
  BLE.setAdvertisedService(beaconService);
  beaconService.addCharacteristic(chirp); 
  beaconService.addCharacteristic(clue);
  beaconService.addCharacteristic(battery);

  chirp.setEventHandler(BLEWritten, chirpCallback);
  battery.setEventHandler(BLERead, batteryCallback);
  
  BLE.addService(beaconService);
  
  chirp.writeValue(doEcho);
  clue.writeValue(2);

  batteryPercent = FuelGauge.percent(); 
  battery.writeValue(String(batteryPercent).c_str());

  BLE.advertise();

  digitalWrite(MOSFET_PIN, LOW);
  
  for(int i = 0; i < 4; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
  }
}

void chirpCallback(BLEDevice central, BLECharacteristic characteristic) {
  chirpCheck();
}

void batteryCallback(BLEDevice central, BLECharacteristic characteristic) {
  FuelGauge.wake();
  batteryPercent = FuelGauge.percent();
  battery.writeValue(String(batteryPercent).c_str());
  FuelGauge.sleep();
}

void chirpCheck() {
  chirp.readValue(doEcho);

  if(doEcho == 1)
  {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(MOSFET_PIN, HIGH);
    
    doEcho = 0;
    chirp.writeValue(doEcho);

    for (int thisNote = 0; thisNote < 8; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(SPEAKER_PIN, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(SPEAKER_PIN);
    }

    digitalWrite(MOSFET_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
  if(doEcho == 2)
  {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(MOSFET_PIN, HIGH);
    
    doEcho = 0;
    chirp.writeValue(doEcho);
    tone(SPEAKER_PIN, melody[0], 1000);
    delay(1000);
    digitalWrite(MOSFET_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }
}

void loop() {
  BLE.poll(1000);
}
