#include <WiFi.h>
#include <ESP8266HTTPClient.h>
#include <MFRC522.h>

#define SS_PIN 4   // SDA pin connected to D4 on Seed Studio Xiao ESP32-C3
#define RST_PIN 5  // RST pin connected to D5 on Seed Studio Xiao ESP32-C3

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define SERVER_ADDRESS "http://your_server_address"  // Replace with your server address

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(115200);
  SPI.begin();        // Initialize SPI communication
  mfrc522.PCD_Init(); // Initialize MFRC522 RFID reader

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // Connect to Wi-Fi network

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  // Check if a new RFID tag is detected
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Get the RFID tag's unique ID
    String rfidTag = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      rfidTag.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      rfidTag.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    // Send the RFID tag to the server for verification
    sendRFIDTag(rfidTag);

    // Halt the program briefly to avoid rapid scanning
    delay(2000);

    // Reset the RFID reader to prepare for the next scan
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

void sendRFIDTag(const String& rfidTag) {
  HTTPClient http;
  String url = SERVER_ADDRESS + "/check_rfid.php?tag=" + rfidTag;

  http.begin(url);  // Specify the server and endpoint
  int httpResponseCode = http.GET();  // Send the HTTP GET request

  if (httpResponseCode == 200) {
    String response = http.getString();  // Get the server response

    if (response == "correct") {
      Serial.println("RFID tag is correct");
      // Transmit the correct status over Wi-Fi (you can customize this part)
      // Example using the Blynk library: Blynk.virtualWrite(V1, "Correct RFID");
    } else if (response == "incorrect") {
      Serial.println("RFID tag is incorrect");
      // Transmit the incorrect status over Wi-Fi (you can customize this part)
      // Example using the Blynk library: Blynk.virtualWrite(V1, "Incorrect RFID");
    } else {
      Serial.println("Invalid server response");
    }
  } else {
    Serial.print("Error: ");
    Serial.println(httpResponseCode);
  }

  http.end();  // Close the connection
}
