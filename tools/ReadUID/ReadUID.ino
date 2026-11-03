/*
  RFID UID reader for the Smart Cart System
  Author: Mohammad Ullah (EEE, RUET)   GitHub: https://github.com/mohammadullah2eee/smart-cart-system

  Upload, open the Serial Monitor at 115200 and tap a tag on the reader.
  The UID is printed in the same format the main sketch uses.
  Copy it into the productUIDs list in SmartCart_ESP32.ino.
*/
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 4
#define SS_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Tap a tag on the reader...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i], HEX);   // same method as the main sketch
  }
  Serial.print("UID: ");
  Serial.println(uid);

  mfrc522.PICC_HaltA();
  delay(500);
}
