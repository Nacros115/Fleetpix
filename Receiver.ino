#include "SparkFun_SinglePairEthernet.h"

SinglePairEthernet adin1110;

byte deviceMAC[6] = {0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA}; // Replace with your receiver MAC address

void rxCallback(byte *data, int dataLen, byte *senderMac) {
    Serial.println("Received UID:");
    for (int i = 0; i < dataLen; i++) {
        Serial.print(data[i], HEX);
        if (i < dataLen - 1) {
            Serial.print(":");
        }
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    if (!adin1110.begin(deviceMAC)) {
        Serial.print("Failed to connect to ADIN1110 MACPHY. Make sure board is connected and pins are defined for target.");
        while (1);
    }
    
    Serial.println("Connected to ADIN1110 MACPHY");

    adin1110.setRxCallback(rxCallback);

    Serial.println("Receiver configured, waiting for UID data...");
    while (adin1110.getLinkStatus() != true);
}

void loop() {
    // No need for explicit loop code since we're using callbacks
}
