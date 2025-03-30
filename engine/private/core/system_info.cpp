#include "core/system_info.h"

std::string SystemInfo::GetPlatform() {

    return std::string(SDL_GetPlatform());
}

std::string SystemInfo::GetDeviceType() {

    return "UNKNOWN";
}

std::string SystemInfo::GetDeviceName() {
    return "UNKNOWN";
}

std::string SystemInfo::GetDeviceModel() {
    return "UNKNOWN";
}

std::string SystemInfo::GetDeviceUniqueIdentifier() {
    return "UNKNOWN";
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
