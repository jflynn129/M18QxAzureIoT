/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   foa.h
*   @brief  functions to manage associations of class functions with class object address.  When an ISR is called
*           external to its class you need an association with the address of the class to access the data members
*           These functions create a linked list of function address to class object address.
*
*   @author James Flynn
*
*   @date   1-Oct-2018
*/

#ifndef __FOA_H__
#define __FOA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <hwlib/hwlib.h>

#ifdef __cplusplus
}
#endif

typedef struct obj_struct_t {
    void *obj;
    int    (*func)(gpio_pin_t pin_state, gpio_irq_trig_t direction);
    obj_struct_t *nxt;
    } obj_struct;

bool foa_insert( void* obj, int (*func)(gpio_pin_t pin_state, gpio_irq_trig_t direction));
void* foa_find(int (*func)(gpio_pin_t pin_state, gpio_irq_trig_t direction));

#endif //__FOA_H__

