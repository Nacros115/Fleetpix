 #include <Wire.h>
#include <Adafruit_PN532.h>
#include <EEPROM.h>
#include <FastLED.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define NUM_LEDS 12
#define LED_PIN 3
int sens = 0;

#define EEPROM_SIZE 64
const int buttonPin = D7;

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

CRGB leds[NUM_LEDS];

unsigned long lastLEDUpdate = 0;
unsigned long LEDUpdateInterval = 100; // Adjust this value to control the speed of the chenillard effect

CRGBPalette16 yellowPalette;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  nfc.begin();

  EEPROM.begin(EEPROM_SIZE);
  Serial.print("Available EEPROM space: ");
  Serial.print(EEPROM_SIZE - 1);
  Serial.println(" bytes");
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  pinMode(buttonPin, INPUT);

  // Define yellow color palette
  yellowPalette = CRGBPalette16(CRGB::Black, CRGB::Yellow);

  // Animated LED boot pattern
  int bootPatternDuration = 3000; // 3 seconds
  unsigned long bootPatternStartTime = millis();

  while (millis() - bootPatternStartTime < bootPatternDuration) {
    float progress = (millis() - bootPatternStartTime) / (float)bootPatternDuration;
    for (int i = 0; i < NUM_LEDS; i++) {
      // Calculate the hue value for each LED in the circle based on the progress
      uint8_t hue = map(int((i + (progress * NUM_LEDS))) % NUM_LEDS, 0, NUM_LEDS - 1, 0, 255);
      // Use the yellow palette to set the color with fading effect
      leds[i] = ColorFromPalette(yellowPalette, hue, 255, LINEARBLEND);
    }
    FastLED.show();
    delay(1); // Adjust the delay to control the animation speed
  }

  setLEDsColor(CRGB::Black); // Turn off all LEDs
  Serial.println("Ready");

  if (checkStoredUID()) {
    Serial.println("Stored UID:");
    printStoredUID();
  } else {
    Serial.println("No stored UID found.");
  }
}


void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState != previousState) {
    if (buttonState == HIGH) {
      currentState = 1; // Button is pressed, enter pairing mode
    } else {
      currentState = 0; // Button is released, enter chenillard mode
    }
  }
  previousState = buttonState;

  if (currentState == 0) {
    chenillard(); // Chenillard effect when the button is released
  } else {
    pairingModeLoop(); // Pairing mode when the button is pressed
  }

  verificationModeLoop();
}

void chenillard() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Violet;

    FastLED.show();
    delay(100); // Adjust the delay to control the speed of the chenillard effect

    leds[i] = CRGB::Black;
  }

  for (int i = NUM_LEDS - 2; i >= 0; i--) {
    leds[i] = CRGB::Violet;

    FastLED.show();
    delay(100); // Adjust the delay to control the speed of the chenillard effect

    leds[i] = CRGB::Black;
  }
}

void pairingModeLoop() {
  Serial.println("Entering pairing mode...");

  delay(1000);
  Serial.println("Ready to pair new NFC tag.");
  Serial.println("Place the tag on the reader...");

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidData.data, &uidData.length)) {
    // Save the new UID only if it's not empty
    if (uidData.length > 0) {
      saveUID(uidData.data, uidData.length);
      Serial.println("Tag saved successfully. The ESP32 will reboot.");
      delay(3000);
      ESP.restart(); // Reboot the ESP32
    } else {
      Serial.println("Tag not detected or is empty. Pairing failed.");
    }
  }
}

void verificationModeLoop() {
  if (isMatchingUID()) {
    if (millis() - lastLEDUpdate >= LEDUpdateInterval) {
      lastLEDUpdate = millis();
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Violet;
      }
      FastLED.show();
      delay(100); // Adjust the delay to control the speed of the chenillard effect
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
      FastLED.show();
    }

    setLEDsColor(CRGB::Green);
    Serial.println("Tag is correct.");
    delay(200);
    setLEDsColor(CRGB::Black);
    delay(200);
  } else {
    if (millis() - lastLEDUpdate >= LEDUpdateInterval) {
      lastLEDUpdate = millis();
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Violet;
      }
      FastLED.show();
      delay(100); // Adjust the delay to control the speed of the chenillard effect
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
      FastLED.show();
    }

    blinkLEDs(2, 200);
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

void setLEDsColor(CRGB color) {
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void blinkLEDs(uint8_t count, uint16_t delayTime) {
  for (uint8_t i = 0; i < count; i++) {
    setLEDsColor(CRGB::Black);
    delay(delayTime);
    setLEDsColor(CRGB::Red);
    delay(delayTime);
  }
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
 
