/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   button.hpp
*   @brief  Button class for detecting & debouncing button preesses. It uses the 'foa' 
*           (function to object) for mapping class location during interrupt calls.
*
*   @author James Flynn
*
*   @date   24-Aug-2018
*/

#ifndef __BUTTON_HPP__
#define __BUTTON_HPP__

#ifndef __HWLIB__
extern "C" {
#include <hwlib/hwlib.h>
}
#endif // __HWLIB__

#include "foa.h"

#pragma GCC diagnostic ignored "-Wpmf-conversions"

class Button {

    protected:
        gpio_handle_t   user_key;
        struct timespec key_press, key_release;
        void            (*br_cb)(int);    //button release callback
        void            (*bp_cb)(void);   //button press callback
        bool            button_press;
        gpio_pin_t      gpio_pin;         //caller specified gpio pin to use

        int irq_callback(gpio_pin_t pin_state, gpio_irq_trig_t direction) {
            Button* obj = (Button*)foa_find((int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Button::irq_callback);
            return button_handler(obj, pin_state, direction);
            }

        int button_handler(Button *data, gpio_pin_t pin_state, gpio_irq_trig_t direction) {
            struct timespec keypress_duration;

	    if (!pin_state) {
                data->button_press = true;
                clock_gettime(CLOCK_MONOTONIC, &data->key_press);
                if( data->bp_cb )
                    data->bp_cb();
                }
            else {
                data->button_press = false;
                clock_gettime(CLOCK_MONOTONIC, &data->key_release);
                if ((data->key_release.tv_nsec - data->key_press.tv_nsec)<0) 
                    keypress_duration.tv_sec = data->key_release.tv_sec - data->key_press.tv_sec-1;
                else 
                    keypress_duration.tv_sec = data->key_release.tv_sec - data->key_press.tv_sec;
   
                if( data->br_cb )
                    data->br_cb(keypress_duration.tv_sec);
                }
            return 0;
            }

    public:
        Button(gpio_pin_t the_gpio, void (*cb)(int)) {
            user_key=0;
            br_cb=cb;
            bp_cb=NULL;
            button_press=false;
            gpio_pin=the_gpio;
            gpio_init( gpio_pin,  &user_key );  //SW3
            gpio_dir(user_key, GPIO_DIR_INPUT);

            foa_insert((void*)this, (int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Button::irq_callback);
            gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, (int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Button::irq_callback);
            }

        ~Button() {
            gpio_deinit( &user_key);
            }

        bool chkButton_press(void) {
            return button_press;
            }

        //allows the user to set a callback for a button press in
        void button_press_cb( void (*buttonpresscb)(void) ) {
            bp_cb = buttonpresscb;
            }
};

#pragma GCC diagnostic warning "-Wpmf-conversions"

#endif  // __BUTTON_HPP__

