#include <Wire.h>
#include <Adafruit_PN532.h>
#include <EEPROM.h>
#include "SparkFun_SinglePairEthernet.h"

SinglePairEthernet adin1110;

#include <seesaw_neopixel.h>
#define PIN 15


#define SDA_PIN 21
#define SCL_PIN 22

int sens = 0;
Adafruit_seesaw ss;
#define NUM_LEDS 16 // Define the number of LEDs in your strip

seesaw_NeoPixel strip = seesaw_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);
#define EEPROM_SIZE 64

int buttonState = 0;
int currentState = 0;
int previousState = 0;

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

struct UIDData {
  uint8_t length;
  uint8_t data[10];
};

UIDData storedUID;
UIDData uidData;


unsigned long lastLEDUpdate = 0;
unsigned long LEDUpdateInterval = 100; // Adjust this value to control the speed of the chenillard effect

byte deviceMAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xC9};
byte destinationMAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA};

//Attached in set-up to be called when the link status changes, parameter is current link status
void linkCallback(bool linkStatus)
{
    digitalWrite(LED_BUILTIN, linkStatus);
}

 
void rainbow(uint8_t wait, uint16_t numCycles) {
  for (uint16_t cycle = 0; cycle < numCycles; cycle++) {
    for (uint16_t j = 0; j < 256; j++) {
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i + j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }
}

void rainbowCycle(uint8_t wait, uint16_t numCycles) {
  for (uint16_t cycle = 0; cycle < numCycles; cycle++) {
    for (uint16_t j = 0; j < 256 * 5; j++) {
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setup() {

  Serial.begin(115200);
  Wire.begin();
  nfc.begin();
  

  if(!strip.begin(0x60)){
    Serial.println("seesaw not found!");
    while(1) delay(10);
  }
  EEPROM.begin(EEPROM_SIZE);
  Serial.print("Available EEPROM space: ");
  Serial.print(EEPROM_SIZE - 1);
  Serial.println(" bytes");

  rainbow(10, 1); // Runs the rainbow animation for 2 cycles
  strip.show(); // Initialize all pixels to 'off'
  Serial.println("Ready");

  if (checkStoredUID()) {
    Serial.println("Stored UID:");
    printStoredUID();
  } else {
    Serial.println("No stored UID found.");
  }
  strip.show(); 
  
if (!adin1110.begin(deviceMAC)) 
    {
      Serial.print("Failed to connect to ADIN1110 MACPHY. Make sure board is connected and pins are defined for target.");
      while(1); //If we can't connect just stop here  
    }
    
    Serial.println("Connected to ADIN1110 MACPHY");

    /* Set up callback */
    adin1110.setLinkCallback(linkCallback);
 
}


void loop() {
 
      verificationModeLoop();
}

void pairingModeLoop() {
  pairingModeAnimation(3); 
  Serial.println("Entering pairing mode...");
  Serial.println("Place the tag on the reader...");

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidData.data, &uidData.length)) {
    // Save the new UID only if it's not empty
    if (uidData.length > 0) {
      saveUID(uidData.data, uidData.length);
      Serial.println("Tag saved successfully. The ESP32 will reboot.");
      
      ESP.restart(); // Reboot the ESP32
    } else {
      Serial.println("Tag not detected or is empty. Pairing failed.");
    }
  }
}

void verificationModeLoop() {
  Serial.println("enter in verification mode");
  if (isMatchingUID()) {
    correctUIDAnimation(3); // Runs the rainbowCycle animation for 3 cycles
    strip.show(); 
     if (adin1110.getLinkStatus()) {
            adin1110.sendData(uidData.data, uidData.length, destinationMAC);
            Serial.println("UID sent over Single Pair Ethernet");
        } else {
            Serial.println("Waiting for link to resume sending");
        }
    Serial.println("Tag is correct.");
    delay(200);
     
  } else {
    wrongUIDAnimation(3);   
    Serial.println("Tag is not correct.");
  }
}

bool checkStoredUID() {
  uint8_t storedLength = EEPROM.read(0);

  if (storedLength > 0 && storedLength <= 10) {
    storedUID.length = storedLength;

    for (uint8_t i = 0; i < storedUID.length; i++) {
      storedUID.data[i] = EEPROM.read(i + 1);
    }

    return true;
  }

  return false;
}

bool isMatchingUID() {
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidData.data, &uidData.length)) {
    if (uidData.length == storedUID.length) {
      for (uint8_t i = 0; i < uidData.length; i++) {
        if (uidData.data[i] != storedUID.data[i]) {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}

void saveUID(const uint8_t* uid, uint8_t uidLength) {
  EEPROM.write(0, uidLength);

  for (uint8_t i = 0; i < uidLength; i++) {
    EEPROM.write(i + 1, uid[i]);
  }

  EEPROM.commit();
}

void printStoredUID() {
  for (uint8_t i = 0; i < storedUID.length; i++) {
    Serial.print(" 0x");
    if (storedUID.data[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(storedUID.data[i], HEX);
  }
  Serial.println();
}
void correctUIDAnimation(uint8_t numCycles) {
  for (uint8_t cycle = 0; cycle < numCycles; cycle++) {
    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0)); // Green color
    }
    strip.show();
    delay(LEDUpdateInterval);
    
    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off LEDs
    }
    strip.show();
    delay(LEDUpdateInterval);
  }
}

void wrongUIDAnimation(uint8_t numCycles) {
  for (uint8_t cycle = 0; cycle < numCycles; cycle++) {
    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0)); // Red color
    }
    strip.show();
    delay(LEDUpdateInterval);
    
    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off LEDs
    }
    strip.show();
    delay(LEDUpdateInterval);
  }
}
 
void pairingModeAnimation(uint8_t numCycles) {
  for (uint8_t cycle = 0; cycle < numCycles; cycle++) {
    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(148, 0, 211)); // Violet color (RGB: 148, 0, 211)
    }
    strip.show();
    delay(LEDUpdateInterval);
    
    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off LEDs
    }
    strip.show();
    delay(LEDUpdateInterval);
  }
}
