
#include "Foundation/Foundation.h"
#import <IOKit/IOKitLib.h>


io_service_t platform_expert(){
    io_service_t platformExpert = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("IOPlatformExpertDevice"));

    return platformExpert;
}

extern "C"{




const char* DeviceName_Injected() {
    NSHost* host = [NSHost currentHost];
    
    if(!host){
        return "UNKNOWN_DEVICE_NAME";
    }
    
    NSString* deviceName = [host localizedName];
    
    if(!deviceName){
        return "UNKNOWN_DEVICE_NAME";
    }
    
    return [deviceName UTF8String];
}

const char* DeviceModel_Injected(){
    io_service_t platformExpert = platform_expert();
    
    if(!platformExpert) {
        return "UNKNOWN_DEVICE";
    }
    
    CFTypeRef modelRef = IORegistryEntryCreateCFProperty(platformExpert, CFSTR("model"), kCFAllocatorDefault, 0);
    IOObjectRelease(platformExpert);

    if(!modelRef) {
        return "UNKNOWN_DEVICE";
    }
    
    NSString* model =  [[NSString alloc] initWithData:(__bridge NSData *)modelRef encoding:NSUTF8StringEncoding];
    
    CFRelease(modelRef);
    
    return strdup([model UTF8String]);
    
}

const char* DeviceUniqueIdentifier_Injected(){
    io_service_t platformExpert = platform_expert();
    
    if(!platformExpert) {
        return "UNKNOWN_UUID";
    }
    
    CFStringRef serialNumber = (CFStringRef)IORegistryEntryCreateCFProperty(platformExpert,CFSTR("IOPlatformSerialNumber"),kCFAllocatorDefault,0);
    IOObjectRelease(platformExpert);
    
    if(!serialNumber){
        return "UNKNOWN_UUID";
    }
    
    NSString* uuid = (__bridge_transfer NSString* )serialNumber;
    
    return [uuid UTF8String];
    
    
    
}

}
