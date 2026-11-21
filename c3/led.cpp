#include <Arduino.h>

#include "led.h"
#include "main.h"

static int ledMode = LED_OFF;
static bool flashShown = false;
static unsigned long flashStart = 0;
static unsigned long flashMs = 0;
static uint8_t flashR, flashG, flashB;
static int lastR = -1, lastG = -1, lastB = -1;

static void writeLED(uint8_t r, uint8_t g, uint8_t b) {
  if (r == lastR && g == lastG && b == lastB) {
    return;
  }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  rgbLedWrite(GPIO_LED, r, g, b);
#else
  neopixelWrite(GPIO_LED, r, g, b);
#endif
  lastR = r;
  lastG = g;
  lastB = b;
}

static bool blinkPhase(void) {
  return (millis() / LED_BLINK_MS) % 2 == 0;
}

void initLED(void) {
  writeLED(0, 0, 0);
}

void setLEDMode(int m) {
  ledMode = m;
}

//Show the color for a while, then return to the current mode
void flashLED(uint8_t r, uint8_t g, uint8_t b, unsigned long ms) {
  flashR = r;
  flashG = g;
  flashB = b;
  flashShown = true;
  flashStart = millis();
  flashMs = ms;
}

//Blocking
void blinkLED(uint8_t r, uint8_t g, uint8_t b, int count) {
  for (int i = 0; i < count; i++) {
    writeLED(r, g, b);
    delay(150);
    writeLED(0, 0, 0);
    delay(150);
  }
}

void loopLED(void) {
  //Compare elapsed time, safe for millis() overflow (about 49.7 days)
  if (flashShown && millis() - flashStart >= flashMs) {
    flashShown = false;
  }

  if (flashShown) {
    writeLED(flashR, flashG, flashB);
    return;
  }

  switch (ledMode) {
  case LED_UNPAIRED:
    writeLED(LED_LEVEL, LED_LEVEL, 0);
    break;
  case LED_PAIRING:
    if (blinkPhase()) {
      writeLED(LED_LEVEL, LED_LEVEL, 0);
    } else {
      writeLED(0, 0, 0);
    }
    break;
  case LED_SCANNING:
    if (blinkPhase()) {
      writeLED(LED_LEVEL, 0, LED_LEVEL);
    } else {
      writeLED(0, 0, 0);
    }
    break;
  case LED_CONNECTED:
    writeLED(0, LED_LEVEL, 0);
    break;
  default:
    writeLED(0, 0, 0);
    break;
  }
}
