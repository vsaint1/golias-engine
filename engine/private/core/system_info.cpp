#include "core/system_info.h"


std::string SystemInfo::platform_name() {

    return std::string(SDL_GetPlatform());
}

bool SystemInfo::is_mobile() {
    const char* platform = SDL_GetPlatform();

    if (SDL_strcmp(platform, "Android") == 0 || SDL_strcmp(platform, "iOS") == 0) {
        return true;
    }

    return false;
}


std::string SystemInfo::get_device_name() {

    return std::string(DeviceName_Injected());
}

std::string SystemInfo::get_device_model() {
    return std::string(DeviceModel_Injected());
}

std::string SystemInfo::get_device_unique_identifier() {
    return std::string(DeviceUniqueIdentifier_Injected());
}

int SystemInfo::get_battery_percentage() {

    int percentage             = 0;
    int sec                    = 0;
    SDL_PowerState power_state = SDL_GetPowerInfo(&sec, &percentage);

    if (power_state == SDL_POWERSTATE_ERROR) {
        LOG_WARN("SYSTEM_INFO: %s", SDL_GetError());
        return 0;
    }

    return percentage;
}
