//
//  PPMediaObject.h
//  Platinum
//
//  Created by Barry Burton on 12/15/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "PPUPnP.h"


#if !defined(_PLT_MEDIA_ITEM_H_)
typedef struct PLT_MediaObject PLT_MediaObject;
#endif
typedef struct PP_MediaObject PP_MediaObject;

@interface PPMediaObject : NSObject {
	PP_MediaObject *object;
}

+ (id)PPMediaObjectWithObject:(PLT_MediaObject *)obj;

- (id)initWithObject:(PLT_MediaObject *)obj;

- (NSString *)name;
- (NSString *)objectId;
- (void)setName:(NSString *)name;
- (void)setOwner:(id)parent;
- (id)getOwner;
- (PP_MediaObject *)getMediaObject;
- (void)setObject:(PLT_MediaObject *)newObject;

@end
