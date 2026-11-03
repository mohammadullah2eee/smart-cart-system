# Build Guide: Smart Cart System

Follow these steps to build the same project.

## 1. What you need

- ESP32 development board and a USB cable
- RC522 RFID reader with 13.56 MHz RFID tags or cards (one per product)
- Load cell with an HX711 amplifier board
- 20x4 I2C LCD
- Buzzer
- Jumper wires and a breadboard
- A computer with the Arduino IDE (free): https://www.arduino.cc/en/software
- A Google account (for Sheets and Apps Script)
- A 2.4 GHz Wi-Fi network (the ESP32 does not use 5 GHz). A phone hotspot works.

## 2. Set up the Arduino IDE

1. Install the ESP32 board package: **Tools → Board → Boards Manager**, search `esp32` and install **esp32 by Espressif Systems**. If you cannot find it, add this address in **File → Preferences** under "Additional boards manager URLs": `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. Choose **Tools → Board → ESP32 Dev Module** (or your exact board).
3. Install these libraries from **Tools → Manage Libraries**:
   - **MFRC522** (by GithubCommunity)
   - **HX711** (HX711 Arduino Library by Bogdan Necula)
   - **LiquidCrystal I2C** (the sketch calls `lcd.begin(20, 4)`, so use a version that supports it on ESP32. If the sketch does not compile, try another LiquidCrystal I2C library.)

`Wire`, `SPI`, `WiFi` and `HTTPClient` come with the ESP32 package.

## 3. Wire the circuit

Wire the modules as in the table in the README (and `images/wiring-diagram.svg`).

| Module | Module pin | ESP32 pin |
|---|---|---|
| RC522 RFID | SDA (SS) | GPIO5 |
| RC522 RFID | RST | GPIO4 |
| RC522 RFID | SCK | GPIO18 |
| RC522 RFID | MOSI | GPIO23 |
| RC522 RFID | MISO | GPIO19 |
| RC522 RFID | 3.3V and GND | 3.3V and GND |
| HX711 | DT (DOUT) | GPIO33 |
| HX711 | SCK | GPIO32 |
| HX711 | VCC and GND | 3.3V and GND |
| LCD (I2C) | SDA | GPIO21 |
| LCD (I2C) | SCL | GPIO22 |
| LCD (I2C) | VCC and GND | 5V and GND |
| Buzzer | (+) | GPIO25 |
| Buzzer | (-) | GND |

- Connect the load cell wires to the HX711 terminals E+, E-, A+ and A-. Wire colours differ between load cells, so check the datasheet of yours.
- The RC522 runs at **3.3V only**. Powering it from 5V can damage it.
- Join all grounds.
- Mount the load cell so that the platform is free to press on it, with nothing touching the platform.

## 4. Check the LCD address

The sketch uses the address `0x27`. Many I2C LCD backpacks use `0x27` and others use `0x3F`. If the screen stays blank, run an I2C scanner sketch to find your address, then change `LiquidCrystal_I2C lcd(0x27, 20, 4);`. Also adjust the small contrast screw on the back of the LCD.

## 5. Calibrate the load cell

1. Open `tools/Calibration_HX711/Calibration_HX711.ino` and upload it.
2. Open the Serial Monitor at **115200**. Keep the platform empty until it says "Tare done".
3. Put a known weight on the platform (for example 500 g).
4. Divide the printed reading by the weight in grams. This is your calibration factor.
5. Put your value into `CALIBRATION_FACTOR` in the main sketch.

Check by weighing a second known weight. If the numbers are off, repeat with a heavier weight.

## 6. Read your RFID tag UIDs

1. Upload `tools/ReadUID/ReadUID.ino`.
2. Open the Serial Monitor at **115200** and tap each tag on the reader.
3. Write down the UID that is printed for each one.
4. Paste them into `productUIDs[]` in the main sketch, in the order of your products.
5. Set `products[]` to the real weight (in grams) and price of each product, in the same order.

Copy the UID exactly as printed. The digit zero and the letter O look alike but are different.

## 7. Set up the Google Sheet and Apps Script

1. Create a new Google Sheet.
2. Open **Extensions → Apps Script**.
3. Replace the code with the contents of `google-apps-script/Code.gs`.
4. Click **Deploy → New deployment**, choose the type **Web app**.
5. Set **Execute as** to **Me** and **Who has access** to **Anyone**, then click **Deploy**. Approve the permissions when Google asks.
6. Copy the **Web app URL**. Keep it private.
7. If you change the script later, deploy a **new version** so the change goes live.

## 8. Upload the main sketch

1. Open `SmartCart_ESP32/SmartCart_ESP32.ino` in the Arduino IDE.
2. Replace `YOUR_WIFI_NAME`, `YOUR_WIFI_PASSWORD` and `PASTE_YOUR_APPS_SCRIPT_WEB_APP_URL_HERE` with your own values.
3. Choose your board and port, then click **Upload**. If you see "Connecting......", hold the **BOOT** button.
4. Open the Serial Monitor at **115200** to watch the messages.

## 9. Test it

1. Keep the platform empty at power-up. The scale sets its zero at start.
2. When the LCD shows "Smart Cart Ready!", tap a product tag. The LCD should show the quantity and the total bill.
3. Check that a new row appears in your Google Sheet.
4. Put the matching product on the platform. No alarm should sound.
5. Put an extra unscanned item on the platform. The buzzer should sound and the LCD should show "Weight Mismatch!".
6. Tap an unknown tag. The LCD should show "Unknown Product!".

## 10. Troubleshooting

- **The LCD is blank:** check the address (step 4), the contrast screw and the 5V supply.
- **The RFID reader does nothing:** check that it has 3.3V and that the SPI wires (SCK, MOSI, MISO, SDA, RST) are correct.
- **Every tag shows "Unknown Product!":** read the UID again with `ReadUID` and paste it exactly (step 6).
- **The weight is wrong or drifts:** calibrate again (step 5). Make sure the platform is steady and the load cell is mounted correctly.
- **The weight reads zero or never changes:** check the HX711 wiring and the load cell wire colours.
- **It is stuck on "Connecting WiFi...":** check the name and password. Use a 2.4 GHz network.
- **The Serial Monitor says data was sent but the Sheet stays empty:** the message prints for any reply, even an error. Check the Web app URL, the deployment settings ("Anyone") and that you deployed the latest version. Also open **Executions** in Apps Script to see errors.
- **Upload fails:** hold the BOOT button while "Connecting..." appears and check the USB cable.

## 11. Ideas to extend it

- Add a **reset or checkout button** to clear the bill.
- Add a **remove item** function and also check when the cart is lighter than expected.
- Store the product list in the Google Sheet and read it from there.
- Add a secret token to the request so only your cart can write to the sheet.
- Make the Wi-Fi connection non-blocking with automatic reconnection.
- Check the HTTP reply code (200) before showing "Data sent successfully!".

## 12. Learn more

- MFRC522 library: https://github.com/miguelbalboa/rfid
- HX711 library: https://github.com/bogde/HX711
- Google Apps Script documentation: https://developers.google.com/apps-script
