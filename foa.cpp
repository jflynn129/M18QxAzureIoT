/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   foa.cpp
*   @brief  functions to manage associations of class functions with class object address.  When an ISR is called
*           external to its class you need an association with the address of the class to access the data members
*           These functions create a linked list of function address to class object address.
*
*   @author James Flynn
*
*   @date   1-Oct-2018
*/

#include <stdlib.h>

#include "foa.h"

static obj_struct *head=NULL;

bool foa_insert( void* obj, void * key)
{
    obj_struct* hptr= head;
    obj_struct* ptr = (obj_struct*) malloc(sizeof(obj_struct));
    if( !ptr )
        return false;
    ptr->obj = obj;
    ptr->key = key;
    ptr->nxt = NULL;
    
    if( hptr == NULL )
        head = ptr;
    else{
        while( hptr->nxt != NULL )
            hptr = hptr->nxt;
        hptr->nxt = ptr;
        }
    return true;
}

void* foa_find(void* key)
{
    obj_struct* hptr = head;
    while( hptr != NULL && hptr->key != key )
        hptr = hptr->nxt;
    if( hptr != NULL )
        return hptr->obj;
    return NULL;
}

