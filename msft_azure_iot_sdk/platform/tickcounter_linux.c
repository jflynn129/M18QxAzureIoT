// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"
#include <stdint.h>
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include <time.h>


typedef struct TICK_COUNTER_INSTANCE_TAG
{
    time_t init_time_value;
    long   current_ms;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    struct timespec presentTime;

    TICK_COUNTER_INSTANCE* tick = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (tick != NULL)
    {
        if( clock_gettime(CLOCK_MONOTONIC, &presentTime) < 0 ) {
            LogError("tickcounter failed: time return INVALID_TIME.");
            free(tick);
            tick = NULL;
        }
        else
        {
            tick->init_time_value = presentTime.tv_nsec;
            tick->current_ms = 0;
        }
    }
    return tick;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
        free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    struct timespec presentTime;
    int result;

    if (tick_counter == NULL || current_ms == NULL) {
        LogError("tickcounter failed: Invalid Arguments.");
        result = __FAILURE__;
        }
    else{
        if( clock_gettime(CLOCK_MONOTONIC, &presentTime) < 0 ) {
            result = __FAILURE__;
            }
        else{
            TICK_COUNTER_INSTANCE* tick_counter_instance = (TICK_COUNTER_INSTANCE*)tick_counter;
            tick_counter_instance->current_ms = (long)(difftime(presentTime.tv_nsec, tick_counter_instance->init_time_value) * 1000);
            *current_ms = tick_counter_instance->current_ms;
            result = 0;
        }
    }

    return result;
}
