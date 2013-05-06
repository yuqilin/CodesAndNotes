//
//  PPUPnP.h
//  Platinum
//
//  Created by Barry Burton on 12/15/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#if !defined(_PLATINUM_H_)
typedef struct PLT_UPnP PLT_UPnP;
#endif

@interface PPUPnP : NSObject {
    PLT_UPnP *upnp;
}

@property (nonatomic, readonly) PLT_UPnP *PLTUPnP;
@property (nonatomic, readonly, getter=isRunning) BOOL running;

+ (PPUPnP *)sharedUPnP;

- (BOOL)start;
- (BOOL)stop;

@end
