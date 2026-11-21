#include <Arduino.h>

#include "ble_central.h"
#include "button.h"
#include "led.h"
#include "main.h"
#include "pairing.h"
#include "state.h"

//State
#define ST_PAIRING_START          0
#define ST_PAIRING_WAIT           1
#define ST_LOAD                   10
#define ST_UNPAIRED               11
#define ST_SCAN_START             20
#define ST_SCANNING               21
#define ST_CONNECT                22
#define ST_INIT_WAIT              23
#define ST_READY                  30
#define ST_RAISE_WAIT             31
#define ST_RETRY_WAIT             40

//Timing
#define ACK_TIMEOUT_MS            2000
#define RETRY_INTERVAL_MS         1000

static int state = ST_LOAD;
static unsigned long stateTime = 0;

//Receive a response "d:<id> <cmd> <OK|NG>" for the command, return true if received
static bool receiveResponse(const char *cmd, bool *ok) {
  char msg[MSG_LEN];
  char c[16], r[8];
  int id;

  while (receiveBLE(msg)) {
    Serial.printf("recv '%s'\n", msg);

    if (sscanf(msg, "d:%d %15s %7s", &id, c, r) != 3) {
      continue;
    }
    //target-3-server returns "unknown NG" for an unsupported command
    if (strcmp(c, cmd) != 0 && strcmp(c, "unknown") != 0) {
      continue;
    }

    *ok = (strcmp(c, cmd) == 0 && strcmp(r, "OK") == 0);
    return true;
  }

  return false;
}

static int getState(void) {
  return state;
}

static unsigned long getStateTime(void) {
  return millis() - stateTime;
}

static void setState(int s) {
  state = s;
  stateTime = millis();
}

static void stPairingStart(void) {
  Serial.printf("pairing: start\n");
  setLEDMode(LED_PAIRING);
  startPairingScan();
  setState(ST_PAIRING_WAIT);
}

static void stPairingWait(void) {
  char addr[ADDR_LEN];
  int rssi;

  if (isScanning()) {
    return;
  }

  if (getPairingResult(addr, sizeof(addr), &rssi) && setPeer(addr)) {
    Serial.printf("pairing: %s (RSSI %d)\n", getPeer(), rssi);
    blinkLED(0, LED_LEVEL, 0, 3);
  } else {
    Serial.printf("pairing: failed, peer is not changed\n");
    flashLED(LED_LEVEL, 0, 0, LED_FLASH_NG_MS);
  }

  setState(ST_LOAD);
}

static void stLoad(void) {
  if (hasPeer()) {
    Serial.printf("peer: %s\n", getPeer());
    setState(ST_SCAN_START);
  } else {
    Serial.printf("peer: (none), hold the button while power on to pair\n");
    setLEDMode(LED_UNPAIRED);
    setState(ST_UNPAIRED);
  }
}

static void stScanStart(void) {
  setLEDMode(LED_SCANNING);
  startPeerScan(getPeer());
  setState(ST_SCANNING);
}

static void stScanning(void) {
  if (isPeerFound()) {
    setState(ST_CONNECT);
  } else if (!isScanning()) {
    setState(ST_SCAN_START);
  }
}

static void stConnect(void) {
  char cmd[16];

  Serial.printf("connect: %s\n", getPeer());
  if (!connectBLE(getPeer())) {
    setState(ST_RETRY_WAIT);
    return;
  }

  //Device ID of target-3-server is not set after boot
  snprintf(cmd, sizeof(cmd), "init %d", TARGET_DEVICE_ID);
  sendBLE(cmd);
  setState(ST_INIT_WAIT);
}

static void stInitWait(void) {
  bool ok;

  if (receiveResponse("init", &ok)) {
    if (ok) {
      Serial.printf("ready\n");
      setLEDMode(LED_CONNECTED);
      setState(ST_READY);
    } else {
      Serial.printf("init: NG\n");
      disconnectBLE();
      setState(ST_RETRY_WAIT);
    }
  } else if (getStateTime() > ACK_TIMEOUT_MS) {
    Serial.printf("init: timeout\n");
    disconnectBLE();
    setState(ST_RETRY_WAIT);
  }
}

static void stReady(bool pressed) {
  if (pressed) {
    sendBLE("raise");
    setState(ST_RAISE_WAIT);
  }
}

static void stRaiseWait(void) {
  bool ok;

  if (receiveResponse("raise", &ok)) {
    if (ok) {
      flashLED(LED_LEVEL, LED_LEVEL, LED_LEVEL, LED_FLASH_OK_MS);
    } else {
      Serial.printf("raise: NG\n");
      flashLED(LED_LEVEL, 0, 0, LED_FLASH_NG_MS);
    }
    setState(ST_READY);
  } else if (getStateTime() > ACK_TIMEOUT_MS) {
    Serial.printf("raise: timeout\n");
    flashLED(LED_LEVEL, 0, 0, LED_FLASH_NG_MS);
    setState(ST_READY);
  }
}

static void stRetryWait(void) {
  setLEDMode(LED_SCANNING);
  if (getStateTime() > RETRY_INTERVAL_MS) {
    setState(ST_SCAN_START);
  }
}

void initState(void) {
  if (isButtonHeld()) {
    setState(ST_PAIRING_START);
  } else {
    setState(ST_LOAD);
  }
}

void loopState(void) {
  bool pressed = buttonPressed();

  if (loopSerial()) {
    stopScan();
    disconnectBLE();
    setState(ST_LOAD);
  }

  if ((getState() == ST_INIT_WAIT || getState() == ST_READY || getState() == ST_RAISE_WAIT) &&
      isBLEDisconnected()) {
    Serial.printf("disconnected\n");
    setState(ST_RETRY_WAIT);
  }

  switch (getState()) {
  case ST_PAIRING_START:
    stPairingStart();
    break;
  case ST_PAIRING_WAIT:
    stPairingWait();
    break;
  case ST_LOAD:
    stLoad();
    break;
  case ST_UNPAIRED:
    break;
  case ST_SCAN_START:
    stScanStart();
    break;
  case ST_SCANNING:
    stScanning();
    break;
  case ST_CONNECT:
    stConnect();
    break;
  case ST_INIT_WAIT:
    stInitWait();
    break;
  case ST_READY:
    stReady(pressed);
    break;
  case ST_RAISE_WAIT:
    stRaiseWait();
    break;
  case ST_RETRY_WAIT:
    stRetryWait();
    break;
  }
}
