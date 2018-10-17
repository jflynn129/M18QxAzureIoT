/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   barometer.hpp
*   @brief  Class for managing an ST LPS25HB sensor.
*
*   @author James Flynn
*
*   @date   24-Aug-2018
*/

#ifndef __BAROMETER_HPP__
#define __BAROMETER_HPP__

#define LPS25HB_SAD      0x5d
#define LPS25HB_WHO_AM_I 0xbd

#include "i2c.hpp"

class Barometer {
  private:
    uint8_t dev_addr;
    i2c     lps25hb_i2c;

    inline uint8_t lps25hb_read_byte(uint8_t reg_addr) {
        return lps25hb_i2c.read(reg_addr);
        }

    inline void lps25hb_write_byte(uint8_t reg_addr, uint8_t value) {
        lps25hb_i2c.write(reg_addr,value);
        }

    float _temp(void) {
        float t = -1;

        if( lps25hb_read_byte(0x27) & 0x01 ){
            uint8_t l  = lps25hb_read_byte(0x2b);
            uint8_t h  = lps25hb_read_byte(0x2c);
            t = ((h << 8 | l )<<((sizeof(int)-2)*8)) >> ((sizeof(int)-2)*8);
            }
        return 42.5 + (float)t/480.0;
        }
            
  public:
    Barometer(uint8_t a) : dev_addr(a),
        lps25hb_i2c(LPS25HB_SAD) 
        {
        uint8_t reg = lps25hb_read_byte(0x20) | 0xb0; //turn device on & configure
        lps25hb_write_byte( 0x20, reg );
        }

    int who_am_i(void) {
      return lps25hb_read_byte(0x0f);
      }

    float get_pressure(void) { //in mbar
        float press = -1;
        if( lps25hb_read_byte(0x27) & 0x02 ){
            uint8_t xl = lps25hb_read_byte(0x28);
            uint8_t l  = lps25hb_read_byte(0x29);
            uint8_t h  = lps25hb_read_byte(0x2a);
            int counts = ((h << 16 | l<<8 | xl)<<((sizeof(int)-3)*8)) >> ((sizeof(int)-3)*8);
            press = (float)counts / 4096.0;
            }
        return press;
        }

    float get_tempC(void) {
        return _temp();
        }

    float get_tempF(void) {
        return (_temp() * (float)1.8+32);  //celcius to Farenheight
        }
};

#endif // __BAROMETER_HPP__

