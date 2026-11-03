/*
  Smart Cart System (ESP32 + RFID + load cell + LCD + Google Sheets)
  ---------------------------------------------------------------
  A prototype smart shopping cart. Scan a product's RFID tag to add it to the
  bill (shown on a 20x4 LCD). A load cell checks that the weight in the cart
  matches the scanned items and a buzzer sounds on a mismatch. Each scan is
  also sent to a Google Sheet through a Google Apps Script web app.

  Author   : Mohammad Ullah
  Dept.    : Electrical and Electronic Engineering (EEE), RUET, Bangladesh
  GitHub   : https://github.com/mohammadullah2eee/smart-cart-system
  Portfolio: https://mohammadullah2eee.github.io

  Before you upload, change these three lines to your own values:
    ssid, password, scriptURL
  and replace the RFID UIDs and product weights/prices with your own.
  Never publish your real Wi-Fi password or Apps Script URL.

  Step-by-step guide and wiring diagram: see README.md and BUILD_GUIDE.md
  in the GitHub repository above. Please keep this header when you share or
  modify the code.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include "HX711.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Pin Definitions
#define RST_PIN 4    // RFID RST pin
#define SS_PIN 5      // RFID SDA pin
#define HX711_DOUT 33 // Load Cell Data pin
#define HX711_SCK 32  // Load Cell Clock pin
#define BUZZER_PIN 25 // Buzzer pin

// I2C Pin Definitions
#define I2C_SDA 21 // SDA pin for I2C
#define I2C_SCL 22 // SCL pin for I2C

// WiFi Credentials
const char* ssid = "YOUR_WIFI_NAME";          // Replace with your WiFi SSID
const char* password = "YOUR_WIFI_PASSWORD";  // Replace with your WiFi Password
const char* scriptURL = "PASTE_YOUR_APPS_SCRIPT_WEB_APP_URL_HERE"; // Replace with Google Apps Script Web App URL

// LCD Display Setup
LiquidCrystal_I2C lcd(0x27, 20, 4); // Adjust I2C address if needed

// RFID Reader Setup
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Load Cell Setup
HX711 scale;
const float CALIBRATION_FACTOR = 1080.0; // Calibration factor for your load cell

// Product Database
struct Product {
  float weight;
  float price;
};
const int NUM_PRODUCTS = 5;
Product products[NUM_PRODUCTS] = {
    {110.0, 50.0},  // Product 1: Weight 100g, Price $50
    {100.0, 100.0}, // Product 2: Weight 200g, Price $100
    {300.0, 150.0}, // Product 3: Weight 300g, Price $150
    {400.0, 200.0}, // Product 4: Weight 400g, Price $200
    {500.0, 250.0}  // Product 5: Weight 500g, Price $250
};

// RFID UIDs
// CHECK: the third UID below contains the letter "O". The reader prints only 0-9 and A-F,
// so a real UID would use the digit zero. Read your own tags with tools/ReadUID and paste them here.
String productUIDs[NUM_PRODUCTS] = {"8FE3331F", "9F59581F", "8FCO7C1F", "9F5AE41F", "C3D1EC2C"};

// Variables
float totalWeight = 0.0;
float totalBill = 0.0;
int count = 0;
float lastWeight = 0.0; // Cache last weight to avoid redundant checks

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  lcd.begin(20, 4);
  lcd.backlight();
  lcd.print("Initializing...");
  delay(2000);

  SPI.begin();
  mfrc522.PCD_Init();
  delay(500);

  scale.begin(HX711_DOUT, HX711_SCK);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.clear();
  lcd.print("Connecting WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Reduced delay for quicker loop
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi Connected!");
  lcd.clear();
  lcd.print("Smart Cart Ready!");
  delay(1000);
  lcd.clear();
}

void loop() {
  // Check RFID
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = getCardUID();
    int productIndex = getProductIndex(uid);

    if (productIndex >= 0) {
      // Add product weight and price
      totalWeight += products[productIndex].weight;
      totalBill += products[productIndex].price;
      count++;

      updateLCD(count, totalBill);

      // Send Data to Google Sheets
      if (WiFi.status() == WL_CONNECTED) {
        sendToGoogleSheets(productIndex + 1, 1); // Send product ID and quantity sold
      }
    } else {
      lcd.clear();
      lcd.print("Unknown Product!");
      delay(1000); // Brief delay for readability
    }

    mfrc522.PICC_HaltA();
  }

  // Check weight periodically
  float currentWeight = scale.get_units();
  if (abs(currentWeight - lastWeight) > 5.0) { // Only react to significant changes
    lastWeight = currentWeight;
    if (currentWeight > totalWeight + 10) { // Mismatch detected
      soundBuzzer();
      lcd.clear();
      lcd.print("Weight Mismatch!");
      delay(1000);
      lcd.clear();
      updateLCD(count, totalBill); // Restore previous display
    }
  }

  delay(10); // Small delay for loop stability
}

String getCardUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  return uid;
}

int getProductIndex(String uid) {
  for (int i = 0; i < NUM_PRODUCTS; i++) {
    if (uid.equalsIgnoreCase(productUIDs[i])) {
      return i;
    }
  }
  return -1;
}

void soundBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300); // Reduced buzzer delay
  digitalWrite(BUZZER_PIN, LOW);
}

void updateLCD(int quantity, float bill) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Product Qty: ");
  lcd.print(quantity);
  lcd.setCursor(0, 1);
  lcd.print("Total Bill: $");
  lcd.print(bill);
}

void sendToGoogleSheets(int productId, int quantity) {
  HTTPClient http;
  http.begin(scriptURL);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"productId\": \"" + String(productId) + "\", \"quantitySold\": " + String(quantity) + "}";
  int httpResponseCode = http.POST(json);

  if (httpResponseCode > 0) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Failed to send data");
  }
  http.end();
}
