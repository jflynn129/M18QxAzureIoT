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
    void *key;
    obj_struct_t *nxt;
    } obj_struct;

bool foa_insert( void* obj, void *key);
void* foa_find(void* key);

#endif //__FOA_H__

