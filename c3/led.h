#ifndef C3_LED_H__
#define C3_LED_H__

#include <stdint.h>

//GPIO
#define GPIO_LED                  2

//LED
#define LED_LEVEL                 64
#define LED_BLINK_MS              250
#define LED_FLASH_OK_MS           150
#define LED_FLASH_NG_MS           1000

#define LED_OFF                   0
#define LED_UNPAIRED              1
#define LED_PAIRING               2
#define LED_SCANNING              3
#define LED_CONNECTED             4

//LED
void initLED(void);
void setLEDMode(int m);
void flashLED(uint8_t r, uint8_t g, uint8_t b, unsigned long ms);
void blinkLED(uint8_t r, uint8_t g, uint8_t b, int count);
void loopLED(void);

#endif /* C3_LED_H__ */
