//
//  PPMediaContainer.mm
//  Platinum
//
//  Created by Barry Burton on 12/15/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "NptTypes.h"
#import "NptResults.h"

#import "Platinum.h"

#import "PP_MediaObject.h"
#import "PPMediaContainer.h"


@interface PPMediaContainer ()

@property (nonatomic, retain) NSMutableArray *list;

@end


@implementation PPMediaContainer

@synthesize list;

- (id)initWithContainer:(PLT_MediaContainer *)obj {
	if ( (self = [super initWithObject:obj]) ) {
		container = obj;
		self.list = [NSMutableArray arrayWithCapacity:10];
	}
    return self;
}

- (NSUInteger)childCount {
	return container->m_ChildrenCount;
}

- (void)setChildCount:(NSUInteger)theChildCount {
	container->m_ChildrenCount = theChildCount;
}

- (BOOL)updateChildCount:(NSUInteger)newChildCount {
	if ( self.childCount == -1 && newChildCount != -1 ) {
		self.childCount = newChildCount;
		return YES;
	} else {
		return NO;
	}
}

- (void)updateDataWithBrowseInfo:(PLT_BrowseInfo *)browseInfo {
	
	/*
	 typedef struct {
	 NPT_String                   object_id;
	 PLT_MediaObjectListReference items;
	 NPT_UInt32                   nr;
	 NPT_UInt32                   tm;
	 NPT_UInt32                   uid;
	 } PLT_BrowseInfo;
	 */

	PP_MediaObject *mediaObject = [self getMediaObject];
	
	mediaObject->childList.Add(browseInfo->items);
	
	[self updateChildCount:browseInfo->tm];
	
	PLT_MediaObjectList::Iterator listIter = browseInfo->items->GetFirstItem();
	while ( listIter ) {
		PPMediaObject *mediaObject = [PPMediaObject PPMediaObjectWithObject:*listIter];
		[list addObject:mediaObject];
		
		listIter++;
	}
}

- (void)dealloc {

    [super dealloc];
}

@end