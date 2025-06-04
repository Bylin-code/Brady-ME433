#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Motor pin definitions
#define M1F 19  // Left motor forward pin
#define M1B 18  // Left motor backward pin
#define M2F 17  // Right motor forward pin
#define M2B 16  // Right motor backward pin

// PWM configuration
#define WRAP_VALUE 12500 // PWM wrap value (125MHz/12500 = 10kHz PWM freq)

// PWM slice IDs for each pin
uint slice_num_m1f;
uint slice_num_m1b;
uint slice_num_m2f;
uint slice_num_m2b;

/**
 * Set motor speed using PWM
 * @param pin GPIO pin to set
 * @param speed Speed from 0.0 (stopped) to 1.0 (full speed)
 */
void set_motor_speed(uint pin, float speed) {
    // Clamp speed between 0 and 1
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;
    
    // Calculate PWM level
    uint16_t level = (uint16_t)(speed * WRAP_VALUE);
    
    // Set PWM level
    pwm_set_chan_level(pwm_gpio_to_slice_num(pin), pwm_gpio_to_channel(pin), level);
}

/**
 * Initialize motor control
 */

void setup_motors(void) {
    // Initialize pins for PWM
    gpio_set_function(M1F, GPIO_FUNC_PWM);
    gpio_set_function(M1B, GPIO_FUNC_PWM);
    gpio_set_function(M2F, GPIO_FUNC_PWM);
    gpio_set_function(M2B, GPIO_FUNC_PWM);
    
    // Get PWM slice numbers for each pin
    slice_num_m1f = pwm_gpio_to_slice_num(M1F);
    slice_num_m1b = pwm_gpio_to_slice_num(M1B);
    slice_num_m2f = pwm_gpio_to_slice_num(M2F);
    slice_num_m2b = pwm_gpio_to_slice_num(M2B);
    
    // Configure PWM
    pwm_set_wrap(slice_num_m1f, WRAP_VALUE);
    pwm_set_wrap(slice_num_m1b, WRAP_VALUE);
    pwm_set_wrap(slice_num_m2f, WRAP_VALUE);
    pwm_set_wrap(slice_num_m2b, WRAP_VALUE);
    
    // Enable PWM
    pwm_set_enabled(slice_num_m1f, true);
    pwm_set_enabled(slice_num_m1b, true);
    pwm_set_enabled(slice_num_m2f, true);
    pwm_set_enabled(slice_num_m2b, true);
    
    // Start with motors stopped
    set_motor_speed(M1F, 0);
    set_motor_speed(M1B, 0);
    set_motor_speed(M2F, 0);
    set_motor_speed(M2B, 0);
}

/**
 * Control robot movement with a single parameter
 * @param control Value from -1.0 to +1.0:
 *               0.0  = full speed forward (both wheels)
 *               +1.0 = pivot right (left wheel full, right wheel stopped)
 *               -1.0 = pivot left (right wheel full, left wheel stopped)
 */
void drive_robot(float control) {
    // Clamp control value between -1 and 1
    if (control < -1.0f) control = -1.0f;
    if (control > 1.0f) control = 1.0f;
    
    float left_speed = 1.0f;  // Left wheel speed (0.0 to 1.0)
    float right_speed = 1.0f; // Right wheel speed (0.0 to 1.0)
    
    // Adjust wheel speeds based on control value
    if (control > 0) {
        // Turning right (reduce right wheel speed)
        right_speed = 1.0f - control;
    } else if (control < 0) {
        // Turning left (reduce left wheel speed)
        left_speed = 1.0f + control; // Note: control is negative here
    }
    
    // Set motor speeds for forward motion
    set_motor_speed(M1F, left_speed);  // Left forward
    set_motor_speed(M1B, 0);           // Left backward off
    set_motor_speed(M2F, right_speed); // Right forward
    set_motor_speed(M2B, 0);           // Right backward off
}

int main() {
    stdio_init_all();
    printf("Line Bot Simple Control Started\n");
    
    // Setup motors and PWM
    setup_motors();
    
    // Example usage demonstration
    while (true) {
        // Go straight (0.0)
        printf("Going straight at full speed\n");
        drive_robot(0.0f);
        sleep_ms(1000);
    
        // Turn slightly right (0.3)
        printf("Turning slightly right\n");
        drive_robot(0.2f);
        sleep_ms(1000);
    
        // Turn hard right (0.8)
        printf("Turning hard right\n");
        drive_robot(0.4f);
        sleep_ms(1000);
        
        // Pivot right (1.0)
        printf("Pivoting right\n");
        drive_robot(0.6f);
        sleep_ms(1000);
    
        // Back to straight (0.0)
        printf("Going straight again\n");
        drive_robot(0.0f);
        sleep_ms(1000);
    
        // Turn slightly left (-0.3)
        printf("Turning slightly left\n");
        drive_robot(-0.2f);
        sleep_ms(1000);
        
        // Turn hard left (-0.8)
        printf("Turning hard left\n");
        drive_robot(-0.4f);
        sleep_ms(1000);
        
        // Pivot left (-1.0)
        printf("Pivoting left\n");
        drive_robot(-0.6f);
        sleep_ms(1000);
    }
}
