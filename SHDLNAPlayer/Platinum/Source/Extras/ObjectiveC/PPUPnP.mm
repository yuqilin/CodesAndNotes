//
//  PltUPnPObject.mm
//  Platinum
//
//  Created by Sylvain on 9/14/10.
//  Copyright 2010 Plutinosoft LLC. All rights reserved.
//

#import "NptTypes.h"
#import "NptResults.h"

#import "Platinum.h"
#import "PltMediaController.h"

#import "PPUPnP.h"

static PPUPnP *sharedInstance = nil;

@implementation PPUPnP

+ (PPUPnP *)sharedUPnP {
	if ( !sharedInstance ) {
		sharedInstance = [[PPUPnP alloc] init];
	}
	return sharedInstance;
}

- (id)init {
    if ( ( self = [super init]) ) {
        upnp = new PLT_UPnP();
		sharedInstance = self;
    }
    return self;
}

- (void)dealloc {
    delete upnp;
    [super dealloc];
}

- (PLT_UPnP*)PLTUPnP {
	return upnp;
}

- (BOOL)isRunning {
    return upnp->IsRunning();
}

- (BOOL)start {
    return upnp->Start() == NPT_SUCCESS;
}

- (BOOL)stop {
    return upnp->Stop() == NPT_SUCCESS;
}

@end
