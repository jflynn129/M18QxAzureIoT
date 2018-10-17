/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   hts221.cpp
*   @brief  this class manages an ST HTS221 sensor, it uses the i2c singleton class to communicate with
*           the device.
*
*   @author James Flynn
*
*   @date   1-Oct-2018
*/

#include <stdint.h>
#include "hts221.hpp"

bool Hts221::getCalibration(void)
{
    uint8_t data;
    uint16_t tmp;

    if( !_hts221_present )
        return false;

    for (int reg=CALIB_START; reg<=CALIB_END; reg++) {
        if ((reg!=CALIB_START+8) && (reg!=CALIB_START+9) && (reg!=CALIB_START+4)) {
            data = hts221_read_byte(reg);
            switch (reg) {
                case CALIB_START:      //0x30
                    _h0_rH = data;
                    break;
                case CALIB_START+1:    //0x31
                    _h1_rH = data;
                    break;
                case CALIB_START+2:    //0x32
                    _T0_degC = data;
                    break;
                case CALIB_START+3:    //0x30
                    _T1_degC = data;
                    break;
                case CALIB_START+5:    //0x35
                    tmp = _T0_degC & 0x00ff;
                    _T0_degC = ((uint16_t)data & 0x0003)<<8;
                    _T0_degC |= tmp;
    
                    tmp = _T1_degC & 0x00ff;;
                    _T1_degC = ((uint16_t)data&0x000C)<<6;
                    _T1_degC |= tmp;
                    break;
                case CALIB_START+6:     //0x36
                    _H0_T0 = data;
                    break;
                case CALIB_START+7:      //0x37
                    _H0_T0 |= ((int)data)<<8;
                    if( _H0_T0 & 0x8000 )
                        _H0_T0 |= 0xffff0000;
                    break;
                case CALIB_START+0xA:    //0x3a
                    _H1_T0 = data;
                    break;
                case CALIB_START+0xB:    //0x3b
                    _H1_T0 |= ((int)data)<<8;
                    if( _H1_T0 & 0x8000 )
                        _H1_T0 |= 0xffff0000;
                    break;
                case CALIB_START+0xC:    //0x3c
                    _T0_OUT = data;
                    break;
                case CALIB_START+0xD:    //0x3d
                    _T0_OUT |= ((int)data)<<8;
                    if( _T0_OUT & 0x8000 )
                        _T0_OUT |= 0xffff0000;
                    break;
                case CALIB_START+0xE:    //0x3e
                    _T1_OUT = data;
                    break;
                case CALIB_START+0xF:    //0x3f
                    _T1_OUT |= ((int)data)<<8;
                    if( _T1_OUT & 0x8000 )
                        _T1_OUT |= 0xffff0000;
                    break;
                case CALIB_START+8:
                case CALIB_START+9:
                case CALIB_START+4:
                    //DO NOTHING
                    break;
    
                // cover any possible error
                default:
                    return false;
                } 
            } 
        }  
    return true;
}

void Hts221::Activate(void)
{
    uint8_t data;

    if( !_hts221_present )
        return;
    data = hts221_read_byte(CTRL_REG1);
    data |= POWER_UP;
    data |= ODR0_SET;
    hts221_write_byte(CTRL_REG1, data);
    getCalibration();
}

void Hts221::Deactivate(void)
{
    uint8_t data;

    if( !_hts221_present )
        return;
    data = hts221_read_byte(CTRL_REG1);
    data &= ~POWER_UP;
    hts221_write_byte(CTRL_REG1, data);
}

bool Hts221::bduActivate(void)
{
    uint8_t data;

    if( !_hts221_present )
        return false;
    data = hts221_read_byte(CTRL_REG1);
    data |= BDU_SET;
    hts221_write_byte(CTRL_REG1, data);

    return true;
}

bool Hts221::bduDeactivate(void)
{
    uint8_t data;

    if( !_hts221_present )
        return false;
    data = hts221_read_byte(CTRL_REG1);
    data &= ~BDU_SET;
    hts221_write_byte(CTRL_REG1, data);
    return true;
}

double Hts221::readHumidity(void)
{
    uint8_t data   = 0;
    uint16_t h_out = 0;
    double _humid  = -1.0;
    double h_tmp   = 0.0;

    if( !_hts221_present )
        return _humid;

    if( !_active )
        Activate();

    while( !(hts221_read_byte(STATUS_REG) & HUMIDITY_READY ) )
        sleep(1);

    data = hts221_read_byte(HUMIDITY_H_REG);
    h_out = data << 8;  // MSB
    data = hts221_read_byte(HUMIDITY_L_REG);
    h_out |= data;      // LSB

    // Decode Humidity
    h_tmp = ((int16_t)(_h1_rH) - (int16_t)(_h0_rH))/2.0;                 // remove x2 multiple

    // Calculate humidity in decimal of grade centigrades i.e. 15.0 = 150.
    _humid  = (double)(((int16_t)h_out - (int16_t)_H0_T0) * h_tmp) / (double)((int16_t)_H1_T0 - (int16_t)_H0_T0);
    h_tmp   = (double)((int16_t)_h0_rH) / 2.0;                       // remove x2 multiple
    _humid += h_tmp;                                                     // provide signed % measurement unit
    return _humid;
}

double Hts221::readTemperature(void)
{
    uint8_t data   = 0;
    uint16_t t_out = 0;
    double deg     = 0.0;
    double _temp   = -1.0;

    if( !_hts221_present )
        return _temp;

    if( !_active )
        Activate();

    while( !((hts221_read_byte(STATUS_REG) & TEMPERATURE_READY) ) )
        sleep(1);

    data= hts221_read_byte(TEMP_H_REG);
    t_out = data  << 8; // MSB
    data = hts221_read_byte(TEMP_L_REG);
    t_out |= data;      // LSB

    // Decode Temperature
    deg    = (double)((int16_t)(_T1_degC) - (int16_t)(_T0_degC))/8.0; // remove x8 multiple

    // Calculate Temperature in decimal of grade centigrades i.e. 15.0 = 150.
    _temp = (double)(((int16_t)t_out - (int16_t)_T0_OUT) * deg) / (double)((int16_t)_T1_OUT - (int16_t)_T0_OUT);
    deg   = (double)((int16_t)_T0_degC) / 8.0;     // remove x8 multiple
    _temp += deg;                                  // provide signed celsius measurement unit

    return _temp;
}

