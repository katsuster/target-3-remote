#include <Arduino.h>
// Bluetooth LE
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "ble_central.h"
#include "main.h"

static BLEUUID serviceUUID(SERVICE_UUID);
static BLEUUID rxUUID(CHARACTERISTIC_UUID_RX);
static BLEUUID txUUID(CHARACTERISTIC_UUID_TX);

static BLEScan *pScan = NULL;
static BLEClient *pClient = NULL;
static BLERemoteCharacteristic *pRxCharacteristic = NULL;
static QueueHandle_t rxQueue = NULL;

static volatile bool scanning = false;
static volatile bool scanFound = false;
static volatile bool deviceConnected = false;
static volatile bool deviceDisconnected = false;

//Scan target (normal mode)
static char scanPeer[ADDR_LEN];
//Scan result (pairing mode)
static bool scanPairing = false;
static volatile int bestRSSI;
static char bestAddr[ADDR_LEN];

// Bluetooth LE Scan Result
class RemoteAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice dev) {
      //target-3-server does not advertise the service UUID, check the name only
      //Use c_str(): getName() and toString() return std::string on core 2.x, String on 3.x
      if (!dev.haveName() || strcmp(dev.getName().c_str(), TARGET_NAME) != 0) {
        return;
      }

      char addr[ADDR_LEN];
      strlcpy(addr, dev.getAddress().toString().c_str(), sizeof(addr));

      if (scanPairing) {
        int rssi = dev.getRSSI();

        if (rssi >= PAIRING_RSSI_MIN && rssi > bestRSSI) {
          bestRSSI = rssi;
          strlcpy(bestAddr, addr, sizeof(bestAddr));
        }
      } else if (strcasecmp(addr, scanPeer) == 0) {
        scanFound = true;
        pScan->stop();
      }
    }
};

// Bluetooth LE Change Connect State
class RemoteClientCallbacks: public BLEClientCallbacks {
    void onConnect(BLEClient *c) {
    }

    void onDisconnect(BLEClient *c) {
      deviceConnected = false;
      deviceDisconnected = true;
    }
};

// Bluetooth LE Receive
static void notifyCallback(BLERemoteCharacteristic *c, uint8_t *data, size_t len, bool isNotify) {
  char msg[MSG_LEN];
  size_t n = min(len, sizeof(msg) - 1);

  memcpy(msg, data, n);
  msg[n] = '\0';
  xQueueSend(rxQueue, msg, 0);
}

static void scanComplete(BLEScanResults results) {
  scanning = false;
}

static void startScan(uint32_t sec) {
  pScan->clearResults();
  scanFound = false;
  scanning = true;
  if (!pScan->start(sec, scanComplete, false)) {
    scanning = false;
  }
}

// Bluetooth LE initialize
void initBLE(void) {
  BLEDevice::init(LOCAL_NAME);

  pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new RemoteAdvertisedDeviceCallbacks(), false);
  pScan->setActiveScan(true);
  pScan->setInterval(100);
  pScan->setWindow(99);

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new RemoteClientCallbacks());

  rxQueue = xQueueCreate(8, MSG_LEN);
}

void startPeerScan(const char *peer) {
  strlcpy(scanPeer, peer, sizeof(scanPeer));
  scanPairing = false;
  startScan(SCAN_SEC);
}

void startPairingScan(void) {
  bestRSSI = -1000;
  bestAddr[0] = '\0';
  scanPairing = true;
  startScan(PAIRING_SCAN_SEC);
}

void stopScan(void) {
  if (scanning) {
    pScan->stop();
    scanning = false;
  }
}

bool isScanning(void) {
  //Scan complete callback is not called if the scan is stopped
  if (scanFound) {
    scanning = false;
  }

  return scanning;
}

bool isPeerFound(void) {
  return scanFound;
}

bool getPairingResult(char *addr, size_t len, int *rssi) {
  if (bestAddr[0] == '\0') {
    return false;
  }

  strlcpy(addr, bestAddr, len);
  *rssi = bestRSSI;

  return true;
}

bool connectBLE(const char *addr) {
  BLERemoteService *pService;
  BLERemoteCharacteristic *pTxCharacteristic;

  pRxCharacteristic = NULL;
  deviceDisconnected = false;

  if (!pClient->connect(BLEAddress(addr))) {
    Serial.printf("connect: failed\n");
    return false;
  }

  pService = pClient->getService(serviceUUID);
  if (pService == NULL) {
    Serial.printf("connect: service not found\n");
    pClient->disconnect();
    return false;
  }

  pRxCharacteristic = pService->getCharacteristic(rxUUID);
  pTxCharacteristic = pService->getCharacteristic(txUUID);
  if (pRxCharacteristic == NULL || pTxCharacteristic == NULL ||
      !pTxCharacteristic->canNotify()) {
    Serial.printf("connect: characteristic not found\n");
    pRxCharacteristic = NULL;
    pClient->disconnect();
    return false;
  }

  xQueueReset(rxQueue);
  pTxCharacteristic->registerForNotify(notifyCallback);
  deviceConnected = true;

  return true;
}

void disconnectBLE(void) {
  if (pClient->isConnected()) {
    pClient->disconnect();
  }
  deviceConnected = false;
  pRxCharacteristic = NULL;
}

bool isBLEDisconnected(void) {
  if (deviceDisconnected) {
    deviceDisconnected = false;
    return true;
  }

  return false;
}

void sendBLE(const char *cmd) {
  if (!deviceConnected || pRxCharacteristic == NULL) {
    return;
  }

  Serial.printf("send '%s'\n", cmd);
  //RX characteristic of target-3-server supports write with response only
  pRxCharacteristic->writeValue((uint8_t *)cmd, strlen(cmd), true);
}

bool receiveBLE(char *msg) {
  return xQueueReceive(rxQueue, msg, 0) == pdTRUE;
}
