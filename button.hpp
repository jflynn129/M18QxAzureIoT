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
        struct timespec key_press, key_release;
        void            (*br_cb)(int);    //button release callback
        void            (*bp_cb)(void);   //button press callback
        bool            button_press;
        bool            button_active;
        gpio_handle_t   key;
        gpio_level_t    pin_state;
        pthread_mutex_t button_mutex;                                                          
        pthread_t       button_thread;
        pthread_cond_t  button_wait;
        static void     *button_handler_thread(void* obj);
        static int irq_callback(gpio_pin_t pin, gpio_irq_trig_t direction) {
            Button* obj = (Button*)foa_find((void*)pin);
            gpio_read(obj->key, &(obj->pin_state));
            pthread_cond_signal(&obj->button_wait);
            return 0;
        }


    public:

        Button(gpio_pin_t the_gpio, void (*cb)(int)) {
            key=0;
            br_cb=cb;
            bp_cb=NULL;
            button_press=false;
            button_active=true;
            gpio_init( the_gpio,  &key );  //SW3
            gpio_dir(key, GPIO_DIR_INPUT);

            pthread_mutex_init(&button_mutex, NULL);
            pthread_cond_init(&button_wait, NULL);
            pthread_create(&button_thread, NULL, button_handler_thread, (void*)this);

            foa_insert((void*)this, (void*)the_gpio);
            gpio_irq_request(key, GPIO_IRQ_TRIG_BOTH, (int (*)(gpio_pin_t, gpio_irq_trig_t))&Button::irq_callback);
            }

        ~Button() { }

        void terminate(void) { 
            int rval = 0;
            button_active = false; 
            pthread_cond_signal(&button_wait);
            pthread_join(button_thread, (void**)&rval);
            gpio_deinit( &key);
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

