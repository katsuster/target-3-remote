#include <Arduino.h>

#include "button.h"

static int btnLast = HIGH;
static int btnStable = HIGH;
static unsigned long btnChanged = 0;
//Ignore the press that is held since boot (e.g. entering pairing mode)
static bool btnArmed = false;

static int readButton(void) {
  if (digitalRead(GPIO_BUTTON) == LOW ||
      digitalRead(GPIO_BUTTON_EXT1) == LOW ||
      digitalRead(GPIO_BUTTON_EXT2) == LOW) {
    return LOW;
  }

  return HIGH;
}

void initButton(void) {
  pinMode(GPIO_BUTTON, INPUT_PULLUP);
  pinMode(GPIO_BUTTON_EXT1, INPUT_PULLUP);
  pinMode(GPIO_BUTTON_EXT2, INPUT_PULLUP);
  delay(10);

  btnLast = readButton();
  btnStable = btnLast;
  btnChanged = millis();
  btnArmed = (btnStable == HIGH);
}

bool isButtonHeld(void) {
  return readButton() == LOW;
}

//Return true only once when the button is pressed, call this every loop
bool buttonPressed(void) {
  int v = readButton();
  unsigned long now = millis();

  if (v != btnLast) {
    btnLast = v;
    btnChanged = now;
  }
  if (now - btnChanged < BUTTON_DEBOUNCE_MS || v == btnStable) {
    return false;
  }

  btnStable = v;
  if (v == HIGH) {
    btnArmed = true;
    return false;
  }

  return btnArmed;
}
