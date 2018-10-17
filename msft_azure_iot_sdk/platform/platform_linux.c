// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"
#ifdef USE_OPENSSL
#include "azure_c_shared_utility/tlsio_openssl.h"
#endif
#if USE_CYCLONESSL
#include "azure_c_shared_utility/tlsio_cyclonessl.h"
#endif
#if USE_WOLFSSL
#include "azure_c_shared_utility/tlsio_wolfssl.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

int platform_init(void)
{
    return 0;
}

const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_get_interface_description(void);

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    // Expected format: "(<runtime name>; <operating system name>; <platform>)"

    STRING_HANDLE result;
    struct utsname nnn;

    if (uname(&nnn) == 0)
    {
        result = STRING_construct_sprintf("(native; %s; %s)", nnn.sysname, nnn.machine);
    }
    else
    {
        LogInfo("WARNING: failed to find machine info.");
        result = STRING_construct("(native; Linux; undefined)");
    }

    return result;
}

void platform_deinit(void)
{
    return;
}
