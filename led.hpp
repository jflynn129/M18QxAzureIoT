/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/* LED Management */

#include <chrono>
#include <thread>
#include <unistd.h>

#ifndef __LED_HPP__
#define __LED_HPP__

#ifndef __HWLIB__
extern "C" {
#include <hwlib/hwlib.h>
}
#endif

#define RESOLUTION 25 //msec

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
            update_leds();
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
            update_leds();
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
            update_leds();
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

        void update_leds(void) {
            while( pthread_mutex_trylock(&led_mutex) )
                pthread_yield();
            if( led_action == NONE || led_action == LOCK || led_action == UNLOCK)
                /* do nothing */;
            else if( led_action == LED_OFF ) 
                set_color_off();
            else if( led_action == LED_ON ) 
                set_color_on(led_color);
            else if( led_action == LED_BLINK ) {
                led_on = !led_on;
                if( led_on ) 
                    set_color_on(led_color);
                else
                    set_color_off();
                }
            pthread_mutex_unlock(&led_mutex);
            }
 
        static void *led_task(void *thread) {
            Led *self = static_cast<Led *>(thread);
            int ret_val = 0;
            int mscnt = 0;

            while ( self->led_active ) {
                if( (self->blink_interval/RESOLUTION) < mscnt) {
                    self->update_leds();
                    mscnt = 0;
                    }
                mscnt++;
                usleep(RESOLUTION*1000);
                }
            pthread_exit(&ret_val);
        }

};

#endif //__LED_HPP__

