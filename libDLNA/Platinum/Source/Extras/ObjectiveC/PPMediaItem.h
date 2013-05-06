//
//  PPMediaItem.h
//  Platinum
//
//  Created by Barry Burton on 12/15/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "PPMediaObject.h"


#if !defined(_PLT_MEDIA_ITEM_H_)
typedef struct PLT_MediaItem PLT_MediaItem;
#endif


@interface PPMediaItem : PPMediaObject {
	PLT_MediaItem *item;
}

- (id)initWithItem:(PLT_MediaItem *)obj;
- (id)initWithMetaData:(NSString *)metaData;

@property (nonatomic, readonly) NSString *metaData;
@property (nonatomic, readonly) NSString *trackName;
@property (nonatomic, readonly) NSString *albumName;
@property (nonatomic, readonly) NSString *artistName;
@property (nonatomic, readonly) NSURL *albumArtURL;
@property (nonatomic, readonly) NSUInteger duration;

@end
