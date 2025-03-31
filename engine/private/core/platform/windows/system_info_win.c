
#include <Windows.h>
#include <stdio.h>

const char* DeviceName_Injected() {
    static char deviceName[256];
    DWORD size;

    if (GetComputerNameA(deviceName, &size)) {
        return deviceName;
    }

    return "UNKNOWN_DEVICE_NAME";
}

const char* DeviceModel_Injected() {
    static char model[256];

    OSVERSIONINFO osInfo;

    RtlZeroMemory(&osInfo, sizeof(OSVERSIONINFO));

    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);


    if (GetVersionEx(&osInfo)) {
        snprintf(model, sizeof(model), "Windows %lu.%lu (Build %lu)", osInfo.dwMajorVersion, osInfo.dwMinorVersion,
                 osInfo.dwBuildNumber);

        return model;
    }


    return "UNKNOWN_DEVICE_MODEL";
}

const char* DeviceUniqueIdentifier_Injected() {
    static char deviceUUID[256];

    HW_PROFILE_INFO hwProfile;

    if(GetCurrentHwProfile(&hwProfile)){
        strncpy(deviceUUID, hwProfile.szHwProfileGuid, sizeof(deviceUUID) - 1);
        deviceUUID[sizeof(deviceUUID) -1] = '\0';
        return deviceUUID;
    }

    return "UNKNOWN_UUID";
}
