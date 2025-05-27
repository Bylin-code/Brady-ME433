#include <stdio.h>
#include "pico/stdlib.h"

// Define GPIO pins for buttons
#define BUTTON_1 18
#define BUTTON_2 19
#define BUTTON_3 20
#define BUTTON_4 21
#define BUTTON_5 17

int main()
{
    stdio_init_all();
    
    // Initialize GPIO pins as inputs with pull-ups enabled
    gpio_init(BUTTON_1);
    gpio_set_dir(BUTTON_1, GPIO_IN);
    gpio_pull_up(BUTTON_1);
    
    gpio_init(BUTTON_2);
    gpio_set_dir(BUTTON_2, GPIO_IN);
    gpio_pull_up(BUTTON_2);
    
    gpio_init(BUTTON_3);
    gpio_set_dir(BUTTON_3, GPIO_IN);
    gpio_pull_up(BUTTON_3);
    
    gpio_init(BUTTON_4);
    gpio_set_dir(BUTTON_4, GPIO_IN);
    gpio_pull_up(BUTTON_4);
    
    gpio_init(BUTTON_5);
    gpio_set_dir(BUTTON_5, GPIO_IN);
    gpio_pull_up(BUTTON_5);
    
    while (true) {
        // Read button states (0 when pressed, 1 when not pressed due to pull-ups)
        bool button1_state = gpio_get(BUTTON_1);
        bool button2_state = gpio_get(BUTTON_2);
        bool button3_state = gpio_get(BUTTON_3);
        bool button4_state = gpio_get(BUTTON_4);
        bool button5_state = gpio_get(BUTTON_5);
        
        // Print button states (showing inverted values so 1 means pressed, 0 means not pressed)
        printf("Button states: [GPIO17: %d] [GPIO18: %d] [GPIO19: %d] [GPIO20: %d] [GPIO21: %d]\n", 
               !button5_state, !button1_state, !button2_state, !button3_state, !button4_state);
        
        // Short delay to avoid flooding the serial output
        sleep_ms(200);
    }
}
