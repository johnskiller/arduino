/**
 * Create a new BLE server.
 */
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include <esp_log.h>
#include <string>
#include "Task.h"

#include "sdkconfig.h"

static char LOG_TAG[] = "MIDIDemo";
BLECharacteristic* pCharacteristic;
uint8_t note = 48;
uint8_t midiPacket[] = {
   0x80,  // header
   0x80,  // timestamp, not implemented 
   0x00,  // status
   0x3c,  // 0x3c == 60 == middle c
   0x00   // velocity
};

class MyTask : public Task {
    void run(void*) {
        while(1){
         note = esp_random() % 13 + 48;
            // note down
            midiPacket[2] = 0x99; // note down, channel 0
            midiPacket[3] = note;
            midiPacket[4] = 127;  // velocity
            pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
            pCharacteristic->notify();
            vTaskDelay(100/portTICK_PERIOD_MS);

            // note up
            midiPacket[2] = 0x89; // note up, channel 0
            midiPacket[3] = note;
            midiPacket[4] = 127;    // velocity
            pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
            pCharacteristic->notify();
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    }
};

MyTask *task;
class MyCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer){
        task->start();
  }

  void onDisconnect(BLEServer* pServer){
        task->stop();
  }
};

class MyCharacteristicCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar){
    uint8_t value[5] = {0};
        memcpy(value, pChar->getValue().c_str(), 5);
    ESP_LOGW(LOG_TAG, "command: %d, note: %d, data: %d, %d, %d", value[2], value[3], value[0], value[1], value[4]);
    }
};

class MainBLEServer: public Task {
  void run(void *data) {
    ESP_LOGD(LOG_TAG, "Starting BLE work!");

    task = new MyTask();
    BLEDevice::init("ESP32");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyCallbacks());

        BLEService* pService = pServer->createService("03b80e5a-ede8-4b33-a751-6ce34ec4c700");
        pCharacteristic = pService->createCharacteristic("7772e5db-3868-4112-a1a9-f2669d106bf3", 
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_WRITE_NR
        );

        pCharacteristic->setCallbacks(new MyCharacteristicCallback());

        pCharacteristic->addDescriptor(new BLE2902());
        pService->start();


    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();


    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);

    ESP_LOGD(LOG_TAG, "Advertising started!");
    delay(portMAX_DELAY);
  }
};


extern "C" void app_main(void)
{
  //esp_log_level_set("*", ESP_LOG_DEBUG);
  MainBLEServer* pMainBleServer = new MainBLEServer();
  pMainBleServer->setStackSize(8000);
  pMainBleServer->start();

} // app_main
