#include <Arduino.h>
#include "EPD.h"

#include "SD.h"

#define SD_MOSI 40
#define SD_MISO 13
#define SD_SCK 39
#define SD_CS 10
SPIClass SD_SPI = SPIClass(HSPI);
extern uint8_t ImageBW[ALLSCREEN_BYTES];

void setup() {
  // Initialization settings, executed only once
  Serial.begin(115200);

  // Screen power
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  pinMode(42, OUTPUT);
  digitalWrite(42, HIGH);
  delay(10);

  // SD card
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS, SD_SPI, 80000000)) {
    Serial.println(F("ERROR: File system mount failed!"));
  } else {
    Serial.printf("SD Size: %lluMB \n", SD.cardSize() / (1024 * 1024));
    char buffer[30]; // Assume the string will not exceed 30 characters, adjust size as needed
    // Use sprintf to format the string and output it to buffer
    int length = sprintf(buffer, "SD Size:%lluMB", SD.cardSize() / (1024 * 1024));
    buffer[length] = '\0';
    EPD_GPIOInit();
    clear_all();
    Serial.println(buffer);

    EPD_ShowString(0, 0, buffer, BLACK, 16);

    EPD_DisplayImage(ImageBW);
    EPD_FastUpdate();
    //    EPD_PartUpdate();
    EPD_Sleep();
  }
}

void loop() {
  // Main loop code, to run repeatedly
  delay(10);
}

void clear_all() {
  // Function to clear the screen content
  EPD_Init();
  EPD_Clear(0, 0, 296, 128, WHITE);
  EPD_ALL_Fill(WHITE);
  EPD_Update();
  EPD_Clear_R26H();
}