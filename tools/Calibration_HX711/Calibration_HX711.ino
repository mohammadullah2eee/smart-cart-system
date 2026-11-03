/*
  HX711 calibration helper for the Smart Cart System
  Author: Mohammad Ullah (EEE, RUET)   GitHub: https://github.com/mohammadullah2eee/smart-cart-system

  1. Upload this sketch and open the Serial Monitor at 115200.
  2. Keep the platform EMPTY until the first line says tare is done.
  3. Put a known weight on the platform (for example 500 g).
  4. Divide the printed reading by the known weight in grams.
     That result is your CALIBRATION_FACTOR for the main sketch.
*/
#include "HX711.h"

#define HX711_DOUT 33
#define HX711_SCK 32

HX711 scale;

void setup() {
  Serial.begin(115200);
  scale.begin(HX711_DOUT, HX711_SCK);
  scale.set_scale();   // factor 1, so readings are raw values
  scale.tare();        // set zero with an empty platform
  Serial.println("Tare done. Now place a known weight on the platform.");
}

void loop() {
  Serial.print("Reading: ");
  Serial.println(scale.get_units(10));
  delay(500);
}
