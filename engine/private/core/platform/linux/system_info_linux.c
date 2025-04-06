#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#ifndef __USE_POSIX
#define __USE_POSIX 1
#endif
#include <limits.h>
#include <sys/utsname.h>

const char* DeviceName_Injected()
{
    static char hostname[HOST_NAME_MAX] = "UNKNOWN_DEVICE_NAME";

    gethostname(hostname, sizeof(hostname));
    return hostname;
}

const char* DeviceModel_Injected()
{
    static char model[256] = "UNKNOWN_DEVICE_MODEL";
    struct utsname sysinfo;

    if (uname(&sysinfo) == 0)
        snprintf(model, sizeof(model), "%s %s (Kernel %s)", sysinfo.sysname, sysinfo.release, sysinfo.version);
    return model;
}


const char* DeviceUniqueIdentifier_Injected() {
    static char deviceUUID[256] = "UNKNOWN_UUID";
    FILE *fp = fopen("/etc/machine-id", "r");

    if (!fp) {
        // non-systemd kernels like artix for example
        fp = fopen("/var/lib/dbus/machine-id", "r");
    }

    if (fp) {
        if (fgets(deviceUUID, sizeof(deviceUUID), fp)) {
            // we remove the newline from the uuid and make it a null char instead
            deviceUUID[strcspn(deviceUUID, "\n")] = '\0';
        }
        fclose(fp);
    }

    return deviceUUID;
}