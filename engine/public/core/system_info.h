#pragma once
#include "helpers/logging.h"

enum BatteryLevel {
    BatteryLevel_Unknown = 0,
};

/*!

   @brief SystemInfo class
    - Get information about the system the app is running on

   @version 0.0.7
*/
class SystemInfo {

public:
    void operator=(const SystemInfo&) = delete;


    /*!

        @brief Get the current platform

        @details
        - `Windows`, `Linux`, `Mac OS X`, `iOS`, `Android`, `Emscripten`.

        @return std::string platform

        @version 0.0.7
    */
    static std::string platform_name();


    /*!

        @brief Check if the app is running on a mobile device

        @return bool

        @version 0.0.7
    */
    static bool is_mobile();

    // TODO: implement
    static std::string get_device_type();

    // TODO: implement
    static std::string get_device_name();

    // TODO: implement
    static std::string get_device_model();

    // TODO: implement
    static std::string get_device_unique_identifier();

    /*!

        @brief Get the battery percentage

        @details This function checks the battery status of the device.
            If the device does not have a battery, it returns -1.
            Otherwise, it returns the battery percentage between (0-100).

        @return int percentage

        @version 0.0.7
    */
    static int get_battery_percentage();


private:
    SystemInfo() = default;
};


extern "C" {

    const char* DeviceName_Injected();

    const char* DeviceModel_Injected();

    const char* DeviceUniqueIdentifier_Injected();
}
