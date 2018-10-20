
#include "button.hpp"


void *Button::button_handler_thread(void *obj)
{
    Button *data = static_cast<Button *>(obj);
    bool   pin_state;
    struct timespec keypress_duration;

    pthread_mutex_lock(&data->button_mutex);
    while( true ) {
        pthread_cond_wait(&data->button_wait, &data->button_mutex);
        pin_state = data->pin_state;
        if (!pin_state) {
            data->button_press = true;
            clock_gettime(CLOCK_MONOTONIC, &data->key_press);
            if( data->bp_cb )
                data->bp_cb();
            }
        else{
            data->button_press = false;
            clock_gettime(CLOCK_MONOTONIC, &data->key_release);
            if ((data->key_release.tv_nsec - data->key_press.tv_nsec)<0) 
                keypress_duration.tv_sec = data->key_release.tv_sec - data->key_press.tv_sec-1;
            else 
                keypress_duration.tv_sec = data->key_release.tv_sec - data->key_press.tv_sec;
       
            if( data->br_cb )
                data->br_cb(keypress_duration.tv_sec);
            }
        } //never return
}

