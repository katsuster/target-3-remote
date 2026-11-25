#ifndef C3_BUTTON_H__
#define C3_BUTTON_H__

//GPIO
#define GPIO_BUTTON               3
//External buttons on target-3-board remote (J3 pin 4: SW1, pin 3: SW2)
#define GPIO_BUTTON_EXT1          4
#define GPIO_BUTTON_EXT2          5

//Timing
#define BUTTON_DEBOUNCE_MS        20

//Button
void initButton(void);
bool isButtonHeld(void);
bool buttonPressed(void);

#endif /* C3_BUTTON_H__ */
