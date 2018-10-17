/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   adc.hpp
*   @brief  A small Adc class for obtaining and returning Adc values
*           note: only created to override the assignment '=' operator
*
*   @author James Flynn
*
*   @date   24-Aug-2018
*/

#ifndef __ADC_HPP__
#define __ADC_HPP__

#ifndef __HWLIB__
extern "C" {
#include <hwlib/hwlib.h>
}
#endif // __HWLIB__

class Adc 
{
    private:
        float        adc_value;
        adc_handle_t the_adc;

    public:
        Adc() : adc_value(0.0), the_adc(0) { adc_init(&the_adc); }
        ~Adc() { adc_deinit(&the_adc); }

      operator float() const {
          float v;
          adc_read(the_adc,&v);
          return v;
          }

      float operator=(Adc) {
          float v;
          adc_read(the_adc,&v);
          return v;
          }
};

#endif  //__ADC_HPP__

