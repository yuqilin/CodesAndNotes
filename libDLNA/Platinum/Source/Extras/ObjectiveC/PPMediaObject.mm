//
//  PPMediaObject.mm
//  Platinum
//
//  Created by Barry Burton on 12/15/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "NptTypes.h"
#import "NptResults.h"

#import "Platinum.h"

#import "PP_MediaObject.h"
#import "PPMediaObject.h"
#import "PPMediaContainer.h"
#import "PPMediaItem.h"




@implementation PPMediaObject


+ (id)PPMediaObjectWithObject:(PLT_MediaObject *)obj {
	PPMediaObject *newObject;
	if ( obj->IsContainer() ) {
		newObject = [[PPMediaContainer alloc] initWithContainer:(PLT_MediaContainer *)obj];
	} else {
		newObject = [[PPMediaItem alloc] initWithItem:(PLT_MediaItem *)obj];
	}
    return newObject;
}

- (id)initWithObject:(PLT_MediaObject *)obj {
	if ( (self = [super init]) ) {
		object = new PP_MediaObject(obj);
	}
    return self;
}

- (void)dealloc {
    delete object;
	
    [super dealloc];
}

- (NSString *)name {
	return [NSString stringWithUTF8String:(char*)object->mediaObject->m_Title];
}

- (NSString *)objectId {
	return [NSString stringWithUTF8String:(char*)object->mediaObject->m_ObjectID];
}

- (void)setName:(NSString *)name {
	PLT_MediaObject *mediaObject = object->mediaObject;
	mediaObject->m_Title = (char *)[name UTF8String];
}

- (void)setOwner:(id)parent {
	object->owner = parent;
}

- (id)getOwner {
	return object->owner;
}

- (PP_MediaObject *)getMediaObject {
	return object;
}

- (void)setObject:(PLT_MediaObject *)newObject {
	object->mediaObject = newObject;
}

@end
