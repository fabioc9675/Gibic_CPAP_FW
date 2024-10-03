#include "bldc/bldc_servo.h"


#define SERVO_PULSE_GPIO             8        // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // Timer resolution
#define SERVO_TIMEBASE_PERIOD        20000    // Timer period   20ms

uint32_t servo_pulse_width_us = 1000; // Pulse width in microseconds


mcpwm_timer_config_t timer_config = {
    .group_id = 0,
    .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
    .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
    .period_ticks = SERVO_TIMEBASE_PERIOD,
    .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
};
mcpwm_timer_handle_t timer = NULL;

/*
 * Configura el operador para generar la señal PWM
 */
mcpwm_operator_config_t operator_config = {
    .group_id = 0, // operator must be in the same group to the timer
};
mcpwm_oper_handle_t oper = NULL;

/*
 * Crea el comparador
 */
mcpwm_comparator_config_t comparator_config = {
    .flags.update_cmp_on_tez = true,
};
mcpwm_cmpr_handle_t comparator = NULL;

/*
 * Crea el generador
 */
mcpwm_gen_handle_t generator = NULL;
mcpwm_generator_config_t generator_config = {
    .gen_gpio_num = SERVO_PULSE_GPIO,
};

void bldc_servo_app(void *pvParameters)
{
    /*
     * Crea el timer
     */ 
    mcpwm_new_timer(&timer_config, &timer);

    /*
     * Crea el operador
     */
    mcpwm_new_operator(&operator_config, &oper);

    /*
     * Conecta el timer con el operador 
     */
    mcpwm_operator_connect_timer(oper, timer);

    /*
     * Crea el comparador
     */
    mcpwm_new_comparator(oper, &comparator_config, &comparator);

    /*
     * Crea el generador
     */
    mcpwm_new_generator(oper, &generator_config, &generator);

    mcpwm_comparator_set_compare_value(comparator, 100);
    
    // go high on counter empty
    mcpwm_generator_set_action_on_timer_event(generator,
                MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
    
    // go low on compare threshold
    mcpwm_generator_set_action_on_compare_event(generator,
                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW));
    
    /*
     * Habilita el timer
     */
    mcpwm_timer_enable(timer);
    mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
    mcpwm_comparator_set_compare_value(comparator, 1000);    

    //servo_pulse_width_us = 1050;
    for(;;) {
        
        
        vTaskDelay(pdMS_TO_TICKS(5000));
        servo_pulse_width_us += 50;
        if (servo_pulse_width_us > 1500) {
            servo_pulse_width_us = 1000;
        }
        ESP_LOGI("main", "Setting pulse width to %ld us", servo_pulse_width_us);
        mcpwm_comparator_set_compare_value(comparator, servo_pulse_width_us);

    }

}