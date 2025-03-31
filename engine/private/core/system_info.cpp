#include "core/system_info.h"


std::string SystemInfo::GetPlatform() {

    return std::string(SDL_GetPlatform());
}

bool SystemInfo::IsMobile() {
    const char* platform = SDL_GetPlatform();

    if (SDL_strcmp(platform, "Android") == 0 || SDL_strcmp(platform, "iOS") == 0) {
        return true;
    }

    return false;
}


std::string SystemInfo::GetDeviceName() {

    return std::string(DeviceName_Injected());
}

std::string SystemInfo::GetDeviceModel() {
    return std::string(DeviceModel_Injected());
}

std::string SystemInfo::GetDeviceUniqueIdentifier() {
    return std::string(DeviceUniqueIdentifier_Injected());
}

int SystemInfo::GetBatteryPercentage() {

    int percentage             = 0;
    int sec                    = 0;
    SDL_PowerState power_state = SDL_GetPowerInfo(&sec, &percentage);

    if (power_state == SDL_POWERSTATE_ERROR) {
        LOG_WARN("SYSTEM_INFO: %s", SDL_GetError());
        return 0;
    }

    return percentage;
}
