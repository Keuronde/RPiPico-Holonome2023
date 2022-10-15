#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "Servomoteur.h"

#define SERVO1 0

void Servomoteur_Init(void){
    gpio_set_function(SERVO1, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO1);
    pwm_config pwm_servo = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&pwm_servo, 254);
    pwm_config_set_phase_correct(&pwm_servo, false);
    pwm_config_set_wrap(&pwm_servo, 9842);


    pwm_init(slice_num, &pwm_servo, true);
    pwm_set_chan_level(slice_num, 0, 984 );


}