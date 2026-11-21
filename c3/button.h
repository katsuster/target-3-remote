#ifndef C3_BUTTON_H__
#define C3_BUTTON_H__

//GPIO
#define GPIO_BUTTON               3
#define GPIO_BUTTON_EXT           0

//Timing
#define BUTTON_DEBOUNCE_MS        20

//Button
void initButton(void);
bool isButtonHeld(void);
bool buttonPressed(void);

#endif /* C3_BUTTON_H__ */
