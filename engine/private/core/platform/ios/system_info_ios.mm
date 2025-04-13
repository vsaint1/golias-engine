#include "Foundation/Foundation.h"
#include "UIKit/UIKit.h"

extern "C" {

    const char* DeviceName_Injected() {
        NSString *deviceName = [[UIDevice currentDevice] name];
        return [deviceName UTF8String];
    }

    const char* DeviceModel_Injected(){
        NSString *deviceModel = [[UIDevice currentDevice] model];
        return [deviceModel UTF8String];
    }

    const char* DeviceUniqueIdentifier_Injected() {
        NSUUID *deviceUUID = [[UIDevice currentDevice] identifierForVendor];
        NSString *uuidString = [deviceUUID UUIDString];
        return [uuidString UTF8String];
    }


}
