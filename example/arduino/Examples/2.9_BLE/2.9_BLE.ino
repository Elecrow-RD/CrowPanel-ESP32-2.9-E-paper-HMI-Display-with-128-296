#include <Arduino.h>
#include "EPD.h"

#include "BLEDevice.h"              // BLE driver library
#include "BLEServer.h"              // BLE Bluetooth server library
#include "BLEUtils.h"               // BLE utility library
#include "BLE2902.h"                // Characteristic descriptor addition library
BLECharacteristic *pCharacteristic;
BLEServer *pServer;
BLEService *pService;
bool deviceConnected = false;
char BLEbuf[32] = {0};

// UART service UUID
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
// UART RX characteristic UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
// UART TX characteristic UUID
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("------> BLE connect .");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("------> BLE disconnect .");
      pServer->startAdvertising(); // Restart advertising
      Serial.println("start advertising");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("------>Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        if (rxValue.find("A") != -1) {
          Serial.print("Rx A!");
        }
        else if (rxValue.find("B") != -1) {
          Serial.print("Rx B!");
        }
        Serial.println();
      }
    }
};
extern uint8_t ImageBW[ALLSCREEN_BYTES];
void setup() {
  // Initialize the screen power
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  // Create the BLE Device
  BLEDevice::init("CrowPanel2-9");
  // Create the BLE server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the service UUID for broadcasting
  pService = pServer->createService(SERVICE_UUID);
  // Create the characteristic for TX
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  // Create the characteristic for RX
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  // Start the BLE service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  // Initialize the e-ink display
  EPD_Init();
  EPD_Clear(0, 0, 296, 128, WHITE);
  EPD_ALL_Fill(WHITE);
  EPD_Update();
  EPD_Clear_R26H();
}
int flag = 0;
void loop() {
  // Main loop code to run repeatedly
  if (deviceConnected) { // After the device is connected, send txValue every second.
    memset(BLEbuf, 0, 32);
    memcpy(BLEbuf, (char*)"Hello BLE APP!", 32);
    pCharacteristic->setValue(BLEbuf);

    pCharacteristic->notify(); // Send the value to the app!
    Serial.print("*** Sent Value: ");
    Serial.print(BLEbuf);
    Serial.println(" ***");
    if (flag != 2)
      flag = 1;
  } else
  {
    if (flag != 4)
      flag = 3;
  }
  if (flag == 1)
  {
    char buffer[30];
    EPD_GPIOInit();
    clear_all();
    strcpy(buffer, "Bluetooth connected");
    strcpy(BLEbuf, "Sent Value:");
    strcat(BLEbuf, "Hello BLE APP!");
    EPD_ShowString(0, 0 + 0 * 20, buffer, BLACK, 16);
    EPD_ShowString(0, 0 + 1 * 20, BLEbuf, BLACK, 16);
    EPD_DisplayImage(ImageBW);
    EPD_FastUpdate();
    //    EPD_PartUpdate();
    EPD_Sleep();

    flag = 2;
  } else if (flag == 3)
  {
    char buffer[30];
    EPD_GPIOInit();
    clear_all();
    strcpy(buffer, "Bluetooth not connected!");
    EPD_ShowString(0, 0 + 0 * 20, buffer, BLACK, 16);
    EPD_DisplayImage(ImageBW);
    EPD_FastUpdate();
    //    EPD_PartUpdate();
    EPD_Sleep();

    flag = 4;
  }
  delay(1000);
}
void clear_all()
{
  EPD_Init();
  EPD_Clear(0, 0, 296, 128, WHITE);
  EPD_ALL_Fill(WHITE);
  EPD_Update();
  EPD_Clear_R26H();
}