//
//  PPMediaDevice.h
//  Platinum
//
//  Created by Barry Burton on 12/16/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "PPUPnP.h"
#import "PPMediaContainer.h"
#import "PPMediaItem.h"


#if !defined(_PP_MEDIA_DEVICE_H_)
typedef struct PP_MediaDevice PP_MediaDevice;
#endif

@class PPMediaController;

@interface PPMediaDevice : NSObject {
	PP_MediaDevice *device;
	CFAbsoluteTime absoluteTime;
	CFAbsoluteTime lastSync;
}

@property (assign) BOOL mute;
@property (assign) NSUInteger volume;
@property (assign) NSUInteger deviceVolume;
@property (retain) PPMediaItem *song;
@property (assign) NSUInteger position;
@property (assign) BOOL isPlaying;
@property (assign) BOOL wasPlaying;
@property (readonly) BOOL supportsFolderURLs;
@property (retain) PPMediaController *controller;
@property (readonly) BOOL isSpeaker;
@property (assign) BOOL stopRequested;
@property (assign) BOOL songFinished;
@property (assign) CFAbsoluteTime lastVolChange;

@property (readonly) PPMediaContainer *rootContainer;
@property (readonly) PP_MediaDevice *deviceData;
@property (readonly) NSString *name;
@property (readonly) NSString *uuid;

@property (assign) id owner;

@property (readonly) NSString *key;

- (BOOL)isEqualToMediaDevice:(PPMediaDevice *)mediaDevice;
- (BOOL)isEqualToPltDevice:(void *)pltDevice;

@end
