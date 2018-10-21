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

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include "foa.h"

#pragma GCC diagnostic ignored "-Wpmf-conversions"

class Button {

    protected:
        gpio_handle_t   user_key;
        struct timespec key_press, key_release;
        void            (*br_cb)(int);    //button release callback
        void            (*bp_cb)(void);   //button press callback
        bool            button_press;
        bool            pin_state;
        gpio_pin_t      gpio_pin;         //caller specified gpio pin to use
        bool            button_active;
        pthread_cond_t  button_wait;
        pthread_mutex_t button_mutex;                                                          
        pthread_t       button_thread;
        static void     *button_handler_thread(void* obj);

        int irq_callback(gpio_pin_t pin_state, gpio_irq_trig_t direction) {
            Button* obj = (Button*)foa_find((int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Button::irq_callback);
            obj->pin_state = (bool)pin_state;
            pthread_cond_signal(&obj->button_wait);
            return 0;
            }

    public:
        Button(gpio_pin_t the_gpio, void (*cb)(int)) {
            user_key=0;
            br_cb=cb;
            bp_cb=NULL;
            button_press=false;
            button_active=true;
            gpio_pin=the_gpio;
            gpio_init( gpio_pin,  &user_key );  //SW3
            gpio_dir(user_key, GPIO_DIR_INPUT);

            pthread_mutex_init(&button_mutex, NULL);
            pthread_cond_init(&button_wait, NULL);
            pthread_create(&button_thread, NULL, button_handler_thread, (void*)this);

            foa_insert((void*)this, (int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Button::irq_callback);
            gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, (int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Button::irq_callback);
            }

        ~Button() { }

        void terminate(void) { 
            int rval = 0;
            button_active = false; 
            pthread_cond_signal(&button_wait);
            pthread_join(button_thread, (void**)&rval);
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

