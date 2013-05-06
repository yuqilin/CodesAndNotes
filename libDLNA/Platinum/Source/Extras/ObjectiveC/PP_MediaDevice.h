//
//  PP_MediaDevice.h
//  Platinum
//
//  Created by Barry Burton on 12/16/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//


#ifndef _PP_MEDIA_DEVICE_H_
#define _PP_MEDIA_DEVICE_H_


class PP_MediaDevice {
public:
	PP_MediaDevice(PLT_DeviceDataReference device) : mediaDevice(device)
													, owner(nil) {

	}
	
	virtual ~PP_MediaDevice() {

	}


	PLT_DeviceDataReference mediaDevice;
	id owner;
};


#endif /* _PP_MEDIA_DEVICE_H_ */
