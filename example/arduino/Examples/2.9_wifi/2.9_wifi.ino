#include <Arduino.h>
#include "EPD.h"

#include <WiFi.h>
String ssid = "yanfa_software";
String password = "yanfa-123456";
extern uint8_t ImageBW[ALLSCREEN_BYTES];

void setup() {
  // Initialization settings, executed only once
  Serial.begin(115200);
  // Screen power
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  char buffer[40];
  EPD_GPIOInit();
  clear_all();
  strcpy(buffer, "WiFi connected");
  EPD_ShowString(0, 0 + 0 * 20, buffer, BLACK, 16);
  strcpy(buffer, "IP address: ");
  strcat(buffer, WiFi.localIP().toString().c_str());
  EPD_ShowString(0, 0 + 1 * 20, buffer, BLACK, 16);
  EPD_DisplayImage(ImageBW);
  EPD_FastUpdate();
  //    EPD_PartUpdate();
  EPD_Sleep();
}

void loop() {
  // Main loop code, to run repeatedly
}

void clear_all() {
  // Function to clear the screen content
  EPD_Init();
  EPD_Clear(0, 0, 296, 128, WHITE);
  EPD_ALL_Fill(WHITE);
  EPD_Update();
  EPD_Clear_R26H();
}