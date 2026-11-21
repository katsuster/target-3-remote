#include <Arduino.h>
#include <esp_mac.h>

#include "ble_central.h"
#include "button.h"
#include "led.h"
#include "main.h"
#include "pairing.h"
#include "state.h"

void setup(void) {
  Serial.begin(115200);

  initLED();
  initButton();
  initPeer();
  initBLE();
  //The State module uses other modules, call it last
  initState();

  Serial.printf("target-3-remote ver." VERSION "\n");

  uint8_t macBT[6];
  esp_read_mac(macBT, ESP_MAC_BT);
  Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                macBT[0], macBT[1], macBT[2], macBT[3], macBT[4], macBT[5]);
}

void loop(void) {
  loopState();
  loopLED();
  delay(5);
}
