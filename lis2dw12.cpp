
#include "lis2dw12.hpp"

float Lis2dw12::lis2dw12_getTemp( void ) 
{
    float   tempC, tempF;
    uint8_t low=0, high=0, hp=0;
    int     i;

    lis2dw12_write_byte(0x3f, 0x00);          // CTRL7: disable interrupts
    lis2dw12_write_byte(0x22, 0x03);          // CTRL3: Enable Single data conversion on command
    while( lis2dw12_read_byte(0x22) & 0x01) 
        sleep(1);
    lis2dw12_write_byte(0x22, 0x00);          // CTRL3: Enable Single data controlled by INT2
    lis2dw12_write_byte(0x3f, 0x20);          // CTRL7: enable interrupts

    hp= lis2dw12_read_byte(0x20) & 0x04; //get performance setting
    low = lis2dw12_read_byte(0x0d);
    high= lis2dw12_read_byte(0x0e);
    i = byte2int(high,low, hp);

    tempC = (float)(i/16.0) + 25.0;
    tempF = (tempC*9.0)/5.0 + 32;
    return tempF;
}

void *Lis2dw12::lis2dw12_int1_thread(void* obj)
{
    Lis2dw12      *data = static_cast<Lis2dw12 *>(obj);
    uint8_t        pos=0, stat=0;

    pthread_mutex_lock(&data->mutex);
    while( true ) {
        pthread_cond_wait(&data->wait, &data->mutex);
        stat = data->lis2dw12_read_byte(0x37);
        if( stat & 0x04 ) {
            pos=data->lis2dw12_read_byte(0x3a) & 0b01111111; 
            if( pos & 0x40 ) {
                data->moved = true;
                if( pos & 0x30 )  //Z threshold
                    data->last_position = (((pos>>4)&0x3) == 1)? FACE_UP:FACE_DOWN;
                if( pos & 0x0c )  //Y threshold
                    data->last_position = (((pos>>2)&0x3) == 1)? FACE_RIGHT:FACE_LEFT;
                if( pos & 0x03 )  //X threshold
                    data->last_position = ((pos&0x3) == 1)? FACE_AWAY:FACE_FORWARD;
                }
            }
        } // will never return...
}

