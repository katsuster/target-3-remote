#ifndef C3_BLE_CENTRAL_H__
#define C3_BLE_CENTRAL_H__

#include <stddef.h>

#define LOCAL_NAME                "Target3Remote"
//Nordic nRF
#define SERVICE_UUID              "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define TARGET_NAME               "Target3"
#define TARGET_DEVICE_ID          1

//Timing
#define SCAN_SEC                  2
#define PAIRING_SCAN_SEC          5
#define PAIRING_RSSI_MIN          -50

//BLE central
void initBLE(void);
void startPeerScan(const char *peer);
void startPairingScan(void);
void stopScan(void);
bool isScanning(void);
bool isPeerFound(void);
bool getPairingResult(char *addr, size_t len, int *rssi);
bool connectBLE(const char *addr);
void disconnectBLE(void);
bool isBLEDisconnected(void);
void sendBLE(const char *cmd);
bool receiveBLE(char *msg);

#endif /* C3_BLE_CENTRAL_H__ */
