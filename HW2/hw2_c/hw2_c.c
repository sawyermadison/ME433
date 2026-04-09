#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define SERVO_PIN 13
#define SERVO_MIN 1400
#define SERVO_MAX 6400

void set_angle(float angle) {
    if (angle < 0)   angle = 0;
    if (angle > 180) angle = 180;
    uint level = SERVO_MIN + (angle / 180.0f) * (SERVO_MAX - SERVO_MIN);
    pwm_set_gpio_level(SERVO_PIN, level);
}

int main() {
    stdio_init_all();

    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_clkdiv(slice_num, 60.0f);
    pwm_set_wrap(slice_num, 50000);
    pwm_set_enabled(slice_num, true);

    while (true) {
        // Sweep 0 → 180
        for (int angle = 0; angle <= 180; angle++) {
            set_angle(angle);
            sleep_ms(15);
        }

        // Sweep 180 → 0
        for (int angle = 180; angle >= 0; angle--) {
            set_angle(angle);
            sleep_ms(15);
        }
    }
}