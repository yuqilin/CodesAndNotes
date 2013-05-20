//
//  PPMediaItem.m
//  Platinum
//
//  Created by Barry Burton on 12/15/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "NptTypes.h"
#import "NptResults.h"

#import "Platinum.h"
#import "PltDidl.h"

#import "PP_MediaObject.h"
#import "PPMediaItem.h"



@implementation PPMediaItem

- (id)initWithItem:(PLT_MediaItem *)obj {
	if ( (self = [super initWithObject:obj]) ) {
		item = obj;
	}
    return self;
}

- (id)initWithMetaData:(NSString *)metaData {
	PLT_MediaObjectListReference songs;
	PLT_MediaObject *song;
	PLT_Didl::FromDidl((char *)[metaData UTF8String], songs);
	songs->Get(0, song);
	self = [self initWithItem:(PLT_MediaItem *)song];
	if ( self ) {
		[self getMediaObject]->childList.Add(songs);
	}
	return self;
}

- (id)init {
	PLT_MediaItem *obj = new PLT_MediaItem();
	return [self initWithItem:obj];
}

- (void)dealloc {
	
    [super dealloc];
}

- (NSString *)metaData {
	return [NSString stringWithUTF8String:(char *)item->m_Didl];
}

- (NSString *)trackName {
	return [super name];
}

- (NSString *)albumName {
	return [NSString stringWithUTF8String:(char *)item->m_Affiliation.album];
}

- (NSString *)artistName {
	if ( item->m_People.artists.GetItemCount() > 0 ) {
		PLT_PersonRole artist;
		
		item->m_People.artists.Get(0, artist);
	
		return [NSString stringWithUTF8String:(char *)artist.name];
	}
	return nil;
}

- (NSURL *)albumArtURL {
	if ( !item->m_ExtraInfo.album_art_uri.IsEmpty() ) {
		NSString *url = [NSString stringWithUTF8String:(char *)item->m_ExtraInfo.album_art_uri];
		return [NSURL URLWithString:url];
	}
	return nil;
}

- (NSUInteger)duration {
	if ( item->m_Resources.GetItemCount() > 0 ) {
		return item->m_Resources[0].m_Duration;
	}
	return 0;
}


@end
