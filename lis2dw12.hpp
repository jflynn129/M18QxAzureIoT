/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

#ifndef __LIS2DW12_HPP__
#define __LIS2DW12_HPP__

#include <signal.h>
#include "i2c.hpp"
#include "foa.h"

#define LIS2DW12_SAD   0x19

#pragma GCC diagnostic ignored "-Wpmf-conversions"

class Lis2dw12 {
    public:
        enum position { FACE_UP=10, FACE_DOWN, FACE_RIGHT, FACE_LEFT, FACE_FORWARD, FACE_AWAY };

        Lis2dw12(gpio_pin_t int1_gpio, gpio_pin_t int2_gpio) : 
            lis2dw12_i2c(LIS2DW12_SAD), 
            int2_pin_state(GPIO_LEVEL_LOW), 
            temp_updated(false),
            last_position(FACE_UP), 
            moved(false),
            lis2dw12_active(true)
            {
            gpio_init(int1_gpio, &int1_pin);
            gpio_init(int2_gpio, &int2_pin);
            gpio_dir(int1_pin, GPIO_DIR_INPUT);   //interrupt input from lis2dw
            gpio_dir(int2_pin, GPIO_DIR_OUTPUT);  //generates interrupt to lis2dw12
            gpio_write( int2_pin,  GPIO_LEVEL_LOW );
    
            pthread_mutex_init(&lis2dw12_mutex, NULL);
            pthread_cond_init(&lis2dw12_wait, NULL);
            pthread_create(&lis2dw12_irq_thread, NULL, lis2dw12_int1_thread, (void*)this);

            foa_insert((void*)this, (int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Lis2dw12::int1_irq_callback);
            gpio_irq_request(int1_pin, GPIO_IRQ_TRIG_RISING, (int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Lis2dw12::int1_irq_callback);

            lis2dw12_write_byte(0x25, 0x00); // CTRL6: Set Full-scale to +/-2g
            lis2dw12_write_byte(0x22, 0x00); // CTRL3: Enable Single data controlled by INT2
            lis2dw12_write_byte(0x23, 0x80); // CTRL4_IN1_PAD_CTRL: Wake-up and Data-read routed to INT1
            lis2dw12_write_byte(0x30, 0x40); // TAP_THS_X: Set 6D threshold
            lis2dw12_write_byte(0x20, 0x30); // CTRL1: Set ODR 25Hz, low-power mode 1 (12-bit)
            lis2dw12_write_byte(0x3f, 0x20); // CTRL7: enable interrupts
            }

        ~Lis2dw12() { }

        void terminate(void) { 
            int rval=0;
            lis2dw12_active=false;
            pthread_cond_signal(&lis2dw12_wait);
            pthread_join(lis2dw12_irq_thread, (void**)&rval);
            gpio_deinit( &int1_pin);
            gpio_deinit( &int2_pin);
            }

        float lis2dw12_getTemp( void );
        position lis2dw12_getPosition(void) {
            return last_position;
            }

        int who_am_i(void) {
            return lis2dw12_read_byte(0x0f);
            }
 
        bool movement_ocured(void) {
            bool move = moved;
            moved = false;
            return move;
            }

    protected:
        int int1_irq_callback(gpio_pin_t pin_state, gpio_irq_trig_t direction) {
            Lis2dw12* obj = (Lis2dw12*)foa_find((int (*)(_gpio_pin_e, _gpio_irq_trig_e))&Lis2dw12::int1_irq_callback);
            pthread_cond_signal(&obj->lis2dw12_wait);
            return 0;
            }
    
        static void lis2dw12_int1_irq_thread(void* data);

    private:
        i2c                    lis2dw12_i2c;
        volatile gpio_level_t  int2_pin_state;
        gpio_handle_t          int2_pin=0, int1_pin=0;
        volatile bool          temp_updated;
        position               last_position;
        bool                   moved;
        bool                   lis2dw12_active;
        pthread_cond_t         lis2dw12_wait;
        pthread_mutex_t        lis2dw12_mutex;                                                          
        pthread_t              lis2dw12_irq_thread;
        static void            *lis2dw12_int1_thread(void* obj);

        inline uint8_t lis2dw12_read_byte(uint8_t reg_addr) {
            return lis2dw12_i2c.read(reg_addr);
            }

        inline void lis2dw12_write_byte(uint8_t reg_addr, uint8_t value) {
            lis2dw12_i2c.write(reg_addr,value);
            }

        inline int byte2int(uint8_t ms, uint8_t ls, bool hp) {
            // incomming data is 2 bytes, in 2's complement form, need to 
            // convert it to a signed int, that is what al the shifting is about
            int v = ((ls | ms << 8)<<((sizeof(int)-2)*8)) >> ((sizeof(int)-2)*8);

            if (!hp)
                return (long)v >> 4;
            else
                return (long)v >> 2;
            }

};

#pragma GCC diagnostic warning "-Wpmf-conversions"

#endif //__LIS2DW12_HPP__

