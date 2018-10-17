/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   i2c.hpp
*   @brief  The i2c class for read/write access to i2c devices
*
*   @author James Flynn
*
*   @date   24-Aug-2018
*/

#ifndef __I2C_HPP__
#define __I2C_HPP__

#include <pthread.h>
#include "i2c_interface.hpp"

class i2c {
    private:
        i2c_interface  *i2c_dev;
        uint8_t         dev_addr;
        pthread_mutex_t i2c_mutex;

    public:
        i2c(uint8_t a) : dev_addr(a), i2c_mutex(PTHREAD_MUTEX_INITIALIZER) {
            i2c_dev = i2c_interface::get_i2c_handle();
            }

        uint8_t read( uint8_t addr ) {
            while( pthread_mutex_trylock(&i2c_mutex) )
                sleep(1);
            unsigned char value_read = i2c_dev->read_i2c(dev_addr, addr);
            pthread_mutex_unlock(&i2c_mutex);
            return value_read;
            }

        void write( uint8_t waddr, uint8_t val ) {
            uint8_t buffer_sent[2] = {waddr, val};
            while( pthread_mutex_trylock(&i2c_mutex) )
                sleep(1);
            i2c_dev->write_i2c(dev_addr, buffer_sent, 2);
            pthread_mutex_unlock(&i2c_mutex);
            }
};

#endif // __I2C_HPP__


