/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/* LED Management */

#include <chrono>
#include <thread>

#ifndef __LED_HPP__
#define __LED_HPP__

#ifndef __HWLIB__
extern "C" {
#include <hwlib/hwlib.h>
}
#endif

class Led {
    public:
        enum Color  { BLACK=0, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE, CURRENT };
        enum Action { LED_OFF=0, LED_ON, LED_BLINK, NONE, LOCK, UNLOCK };

        Led(gpio_pin_t r, gpio_pin_t b, gpio_pin_t g) : red_pin(r),
                                                        blue_pin(b),
                                                        green_pin(g),
                                                        led_mutex(PTHREAD_MUTEX_INITIALIZER),
                                                        red_led(0),
                                                        blue_led(0),
                                                        green_led(0),
                                                        blink_interval(500),
                                                        led_action(NONE),
                                                        led_color(BLACK),
                                                        led_on(false),
                                                        led_active(true)
            {
            gpio_init( red_pin,  &red_led );
            gpio_init( green_pin, &green_led );
            gpio_init( blue_pin, &blue_led );
        
            gpio_dir(red_led,   GPIO_DIR_OUTPUT);
            gpio_dir(green_led, GPIO_DIR_OUTPUT);
            gpio_dir(blue_led,  GPIO_DIR_OUTPUT);
        
            pthread_create( &led_thread, NULL, led_task, (void*)this);
            }

        ~Led() { }

        void terminate(void) {
            int tvalue;
            led_active = false; 
            pthread_join(led_thread, (void**)&tvalue);
            set_color_on(BLACK);
            gpio_deinit( &red_led);
            gpio_deinit( &green_led);
            gpio_deinit( &blue_led);
            }

        int  set_interval(int i) {
            int k = blink_interval;
            blink_interval = i;
            return k;
            }

        Color color( Color color ) {
            while( pthread_mutex_trylock(&led_mutex) )
                pthread_yield();
            if( color != CURRENT )
                led_color=color;
            pthread_mutex_unlock(&led_mutex);
            return led_color;
            }

        Action action( Action act) {
            if( led_action == LOCK && act != UNLOCK )
                return led_action;
            while( pthread_mutex_trylock(&led_mutex) )
                pthread_yield();
            Action last_action = led_action;
            led_action=act;
            if( act == LOCK )
                set_color_on(led_color);

            pthread_mutex_unlock(&led_mutex);
            return last_action;
            }
            
        Action action( Action act, Color color) {
            if( led_action == LOCK && act != UNLOCK )
                return led_action;
            while( pthread_mutex_trylock(&led_mutex) )
                pthread_yield();
            Action last_action = led_action;
            led_action=act;
            if( act == LOCK )
                set_color_on(color);
            else if( color != CURRENT ) 
                led_color=color;
            pthread_mutex_unlock(&led_mutex);
            return last_action;
            }

    private:
        pthread_t       led_thread;
        gpio_pin_t      red_pin,blue_pin,green_pin;

    protected:
        pthread_mutex_t led_mutex;
        gpio_handle_t   red_led,blue_led,green_led;
        int             blink_interval; //msec
        Action          led_action;
        Color           led_color;
        bool            led_on;
        bool            led_active;

        void set_color_off(void) {
            gpio_write( red_led,   GPIO_LEVEL_LOW );
            gpio_write( green_led, GPIO_LEVEL_LOW );
            gpio_write( blue_led,  GPIO_LEVEL_LOW );
            }

        void set_color_on(Color c) {
            gpio_write( red_led,   (c&0x4)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
            gpio_write( green_led, (c&0x2)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
            gpio_write( blue_led,  (c&0x1)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
            }

        static void *led_task(void *thread) {
            Led *self = static_cast<Led *>(thread);
            int ret_val = 0;

            while ( self->led_active ) {
                while( pthread_mutex_trylock(&self->led_mutex) )
                    pthread_yield();
                if( self->led_action == NONE || self->led_action == LOCK || self->led_action == UNLOCK)
                    /* do nothing */;
                else if( self->led_action == LED_OFF ) 
                    self->set_color_off();
                else if( self->led_action == LED_ON ) 
                    self->set_color_on(self->led_color);
                else if( self->led_action == LED_BLINK ) {
                    self->led_on = !self->led_on;
                    if( self->led_on ) 
                        self->set_color_on(self->led_color);
                    else
                        self->set_color_off();
                    }
                pthread_mutex_unlock(&self->led_mutex);
                std::this_thread::sleep_for(std::chrono::milliseconds(self->blink_interval));
                }
            pthread_exit(&ret_val);
        }

};

#endif //__LED_HPP__

