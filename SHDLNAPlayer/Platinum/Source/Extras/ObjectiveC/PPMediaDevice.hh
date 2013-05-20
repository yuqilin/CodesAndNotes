//
//  PPMediaDevice.hh
//  Platinum
//
//  Created by Barry Burton on 2/2/11.
//  Copyright 2011 Gravity Mobile. All rights reserved.
//

#import "PP_MediaDevice.h"
#import "PPMediaDevice.h"

@interface PPMediaDevice ()

+ (PPMediaDevice *)mediaDeviceForPltDevice:(PLT_DeviceDataReference)deviceData;
+ (PPMediaDevice *)lookupMediaDeviceForPltDevice:(PLT_DeviceData *)deviceData;

@end
