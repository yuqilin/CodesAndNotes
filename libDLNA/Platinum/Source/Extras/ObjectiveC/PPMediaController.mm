//
//  PPMediaController.mm
//  Platinum
//
//  Created by Barry Burton on 12/15/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "NptTypes.h"
#import "NptResults.h"

#import "Platinum.h"
#import "PltMediaBrowser.h"
#import "PltMediaController.h"

#import "PPMediaController.h"


#import "PPMediaDevice.hh"

#import "PP_MediaObject.h"
#import "PPMediaObject.h"

#import "PltStateVariable.h"
#import "PltDidl.h"



class PP_MediaController : public PLT_MediaBrowserDelegate, public PLT_MediaControllerDelegate {
public:
	PP_MediaController(PPMediaController *parent, PLT_UPnP *upnp) : master(parent) {
		controlPointRef = PLT_CtrlPointReference(new PLT_CtrlPoint());
		mediaController = new PLT_MediaController(controlPointRef, this);
		mediaBrowser = new PLT_MediaBrowser(controlPointRef, this);
		upnp->AddCtrlPoint(controlPointRef);
	}
	
	virtual ~PP_MediaController() {
		
	}
	
	virtual bool OnMSAdded(PLT_DeviceDataReference& device) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		PPMediaDevice *mediaDevice = [[PPMediaDevice mediaDeviceForPltDevice:device] retain];

		[master.delegate performSelectorOnMainThread:@selector(shouldAddServer:) withObject:mediaDevice waitUntilDone:NO];
		
		[pool release];
		
		return true;
	}
	
	virtual void OnMSRemoved(PLT_DeviceDataReference& device) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		PPMediaDevice *mediaDevice = [[PPMediaDevice mediaDeviceForPltDevice:device] retain];
		
		[master.delegate performSelectorOnMainThread:@selector(didRemoveServer:) withObject:mediaDevice waitUntilDone:NO];
		
		[pool release];
	}
	
	virtual void OnMSStateVariablesChanged(PLT_Service*                  service,
										   NPT_List<PLT_StateVariable*>* vars) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		[pool release];
	}
	
	virtual void OnBrowseResult(NPT_Result               res,
								PLT_DeviceDataReference& device,
								PLT_BrowseInfo*          info,
								void*                    userdata) {
		
		if ( !info ) {
			return;
		}
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		PPMediaObject *user = (PPMediaObject *)userdata;

		if ( [user isKindOfClass:[PPMediaContainer class]] ) {
			[(PPMediaContainer *)user updateDataWithBrowseInfo:info];
		} else {
			NSLog(@"ERROR: Browse Response but not for Container.");
		}
		
		// Call delegate with new Objects
		[master.delegate performSelectorOnMainThread:@selector(browseDidRespond:) withObject:user waitUntilDone:NO];
		
		[pool release];
	}
	
	virtual void OnSearchResult(NPT_Result               res,
						PLT_DeviceDataReference& device,
						PLT_BrowseInfo*          info,
						void*                    userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

		[pool release];
	}





	// PLT_MediaControllerDelegate methods
	virtual bool OnMRAdded(PLT_DeviceDataReference&  device ) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		PPMediaDevice *mediaDevice = [[PPMediaDevice mediaDeviceForPltDevice:device] retain];
		
		[master.delegate performSelectorOnMainThread:@selector(shouldAddSpeaker:) withObject:mediaDevice waitUntilDone:NO];
		
		[pool release];
		
		return true;
	}
	
	virtual void OnMRRemoved(PLT_DeviceDataReference&  device ) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		PPMediaDevice *mediaDevice = [[PPMediaDevice mediaDeviceForPltDevice:device] retain];
		
		[master.delegate performSelectorOnMainThread:@selector(didRemoveSpeaker:) withObject:mediaDevice waitUntilDone:NO];
		
		[pool release];
	}
	
	virtual void OnMRStateVariablesChanged(PLT_Service*                   service, 
                                           NPT_List<PLT_StateVariable*>*  vars) {

		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		PPMediaDevice *speaker = [PPMediaDevice lookupMediaDeviceForPltDevice:service->GetDevice()];
		
		if ( speaker ) {
			NPT_List<PLT_StateVariable *>::Iterator listIter = vars->GetFirstItem();
			
			while ( listIter ) {
				PLT_StateVariable *item = *listIter;
				
				if ( item->GetName().Compare("Volume", true) == 0 ) {
					speaker.deviceVolume = [[NSString stringWithUTF8String:(char *)item->GetValue()] integerValue];
					CFAbsoluteTime timeNow = CFAbsoluteTimeGetCurrent();
					if ( timeNow - speaker.lastVolChange > 1 ) {
						speaker.volume = speaker.deviceVolume;
					}
				} else if ( item->GetName().Compare("Mute", true) == 0 ) {
					speaker.mute = [[NSString stringWithUTF8String:(char *)item->GetValue()] boolValue];
				} else if ( item->GetName().Compare("TransportState", true) == 0 ) {
					speaker.wasPlaying = speaker.isPlaying;
					speaker.isPlaying = item->GetValue().Compare("PLAYING", true) == 0 ? YES : NO;
					if ( speaker.wasPlaying == speaker.isPlaying ) {
						speaker.wasPlaying = NO;
					}
					if ( speaker.stopRequested ) {
						speaker.stopRequested = NO;
						speaker.wasPlaying = NO;
						if ( !speaker.isPlaying ) {
							speaker.position = 0;
						}
					}
					if ( item->GetValue().Compare("STOPPED", true) == 0 ) {
						speaker.position = 0;
					}
				} else if ( item->GetName().Compare("RelativeTimePosition", true) == 0 ) {
					NPT_UInt32 relTime = 0;
					PLT_Didl::ParseTimeStamp(item->GetValue(), relTime);
					if ( !speaker.stopRequested ) {
						speaker.position = relTime;
					}
				} else if ( item->GetName().Compare("CurrentTrackURI", true) == 0 ) {
					// TODO remain tolerant of remote changes
				} else if ( item->GetName().Compare("AVTransportURI", true) == 0 ) {
					// TODO remain tolerant of remote changes
				} else if ( item->GetName().Compare("CurrentTrackMetaData", true) == 0 ) {	
					// TODO remain tolerant of remote changes
				} else if ( item->GetName().Compare("AVTransportURIMetaData", true) == 0 ) {
					// TODO remain tolerant of remote changes
				}
				
				listIter++;
			}

			[master.delegate performSelectorOnMainThread:@selector(speakerUpdated:) withObject:speaker waitUntilDone:NO];
		}
		
		[pool release];
	}
	
    // AVTransport
	virtual void OnGetCurrentTransportActionsResult(NPT_Result                res,
											PLT_DeviceDataReference&  device,
											PLT_StringList*           actions,
											void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		[pool release];
	}
	
	virtual void OnGetDeviceCapabilitiesResult(NPT_Result                res,
									   PLT_DeviceDataReference&  device,
									   PLT_DeviceCapabilities*   capabilities,
									   void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		/*
		typedef struct {
			PLT_StringList play_media;
			PLT_StringList rec_media;
			PLT_StringList rec_quality_modes;
		} PLT_DeviceCapabilities;
		*/
		
		[pool release];
	}
	
	virtual void OnGetMediaInfoResult(NPT_Result                res,
							  PLT_DeviceDataReference&  device,
							  PLT_MediaInfo*            info,
							  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		/*
		typedef struct {
			NPT_UInt32    num_tracks;
			NPT_TimeStamp media_duration;
			NPT_String    cur_uri;
			NPT_String    cur_metadata;
			NPT_String    next_uri;
			NPT_String    next_metadata;
			NPT_String    play_medium;
			NPT_String    rec_medium;
			NPT_String    write_status;
		} PLT_MediaInfo;
		 */
		
		if ( info ) {
			PPMediaDevice *speaker = (PPMediaDevice *)userdata;
			if ( 0 && !speaker.song && !info->cur_metadata.IsEmpty() ) {
				speaker.song = [[PPMediaItem alloc] initWithMetaData:[NSString stringWithUTF8String:(char *)info->cur_metadata]];
			}
			[master.delegate performSelectorOnMainThread:@selector(speakerUpdated:) withObject:speaker waitUntilDone:NO];
		}
		
		[pool release];
	}
	
	virtual void OnGetPositionInfoResult(NPT_Result                res,
								 PLT_DeviceDataReference&  device,
								 PLT_PositionInfo*         info,
								 void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		/*
		typedef struct {
			NPT_UInt32    track;
			NPT_TimeStamp track_duration;
			NPT_String    track_metadata;
			NPT_String    track_uri;
			NPT_TimeStamp rel_time;
			NPT_TimeStamp abs_time;
			NPT_Int32     rel_count;
			NPT_Int32     abs_count;
		} PLT_PositionInfo;
		 */
		
		if ( info ) {
			PPMediaDevice *speaker = (PPMediaDevice *)userdata;
			if ( !speaker.song && !info->track_metadata.IsEmpty() ) {
				speaker.song = [[PPMediaItem alloc] initWithMetaData:[NSString stringWithUTF8String:(char *)info->track_metadata]];
			}
			if ( !speaker.stopRequested ) {
				speaker.position = info->rel_time.ToSeconds();
			}
			[master.delegate performSelectorOnMainThread:@selector(speakerUpdated:) withObject:speaker waitUntilDone:NO];
		}
		
		[pool release];
	}
	
	virtual void OnGetTransportInfoResult(NPT_Result                res,
								  PLT_DeviceDataReference&  device,
								  PLT_TransportInfo*        info,
								  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		/*
		typedef struct {
			NPT_String cur_transport_state;
			NPT_String cur_transport_status;
			NPT_String cur_speed;
		} PLT_TransportInfo;
		 */
		if ( info ) {
			PPMediaDevice *speaker = (PPMediaDevice *)userdata;
			
			BOOL isPlaying = info->cur_transport_state.Compare("PLAYING", true) == 0 ? YES : NO;
	
			[speaker setIsPlaying:isPlaying];
		}
		
		[pool release];
	}
	
	virtual void OnGetTransportSettingsResult(NPT_Result                res,
									  PLT_DeviceDataReference&  device,
									  PLT_TransportSettings*    settings,
									  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		/*
		typedef struct {
			NPT_String play_mode;
			NPT_String rec_quality_mode;
		} PLT_TransportSettings;
		 */
	
		[pool release];
	}
	
	virtual void OnNextResult(NPT_Result                res,
					  PLT_DeviceDataReference&  device,
					  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
		[pool release];
	}
	
	virtual void OnPauseResult(NPT_Result                res,
					   PLT_DeviceDataReference&  device,
					   void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		if ( res == NPT_SUCCESS ) {
			PPMediaDevice *speaker = (PPMediaDevice *)userdata;
			[master updatePositionInfoForSpeaker:speaker];
		}
		
		[pool release];
	}  
	
	virtual void OnPlayResult(NPT_Result                res,
					  PLT_DeviceDataReference&  device,
					  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		if ( res == NPT_SUCCESS ) {
			PPMediaDevice *speaker = (PPMediaDevice *)userdata;
			[master updatePositionInfoForSpeaker:speaker];
		}
		
		[pool release];
	}
	
	virtual void OnPreviousResult(NPT_Result                res,
						  PLT_DeviceDataReference&  device,
						  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		[pool release];
	}
	
	virtual void OnSeekResult(NPT_Result                res,
					  PLT_DeviceDataReference&  device,
					  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
		[pool release];
	}
	
	virtual void OnSetAVTransportURIResult(NPT_Result                res,
								   PLT_DeviceDataReference&  device,
								   void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		if ( NPT_SUCCESS == res ) {
			NSLog(@"Set");
			
			PPMediaDevice *speaker = (PPMediaDevice *)userdata;
			[master playSpeaker:speaker];
			[master updatePositionInfoForSpeaker:speaker];
		}
	
		[pool release];
	}
	
	virtual void OnSetPlayModeResult(NPT_Result                res,
							 PLT_DeviceDataReference&  device,
							 void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
		[pool release];
	}
	
	virtual void OnStopResult(NPT_Result                res,
					  PLT_DeviceDataReference&  device,
					  void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

		[pool release];
	}
	
    // ConnectionManager
	virtual void OnGetCurrentConnectionIDsResult(NPT_Result               res,
										 PLT_DeviceDataReference& device,
										 PLT_StringList*          ids,
										 void*                    userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
		[pool release];
	}
	
	virtual void OnGetCurrentConnectionInfoResult(NPT_Result               res,
										  PLT_DeviceDataReference& device,
										  PLT_ConnectionInfo*      info,
										  void*                    userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		/*
		typedef struct {
			NPT_UInt32 rcs_id;
			NPT_UInt32 avtransport_id;
			NPT_String protocol_info;
			NPT_String peer_connection_mgr;
			NPT_UInt32 peer_connection_id;
			NPT_String direction;
			NPT_String status;
		} PLT_ConnectionInfo;
		*/
	
		[pool release];
	}
	
	virtual void OnGetProtocolInfoResult(NPT_Result               res,
								 PLT_DeviceDataReference& device,
								 PLT_StringList*          sources,
								 PLT_StringList*          sinks,
								 void*                    userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
		[pool release];
	}
	
    // RenderingControl
	virtual void OnSetMuteResult(NPT_Result               res,
						 PLT_DeviceDataReference& device,
						 void*                    userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
		[pool release];
	}
	
	virtual void OnGetMuteResult(NPT_Result                res,
						 PLT_DeviceDataReference&  device,
						 const char*               channel,
						 bool                      mute,
						 void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		PPMediaDevice *speaker = (PPMediaDevice *)userdata;
		speaker.mute = mute;
		
		[master.delegate performSelectorOnMainThread:@selector(speakerUpdated:) withObject:speaker waitUntilDone:NO];
	
		[pool release];
	}
	
	virtual void OnSetVolumeResult(NPT_Result                res,
						   PLT_DeviceDataReference&  device,
						   void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

		if ( res == NPT_SUCCESS ) {
			CFAbsoluteTime timeNow = CFAbsoluteTimeGetCurrent();
			PPMediaDevice *speaker = (PPMediaDevice *)userdata;
			if ( timeNow - speaker.lastVolChange > 1 ) {
				speaker.volume = speaker.deviceVolume;
				
				[master.delegate performSelectorOnMainThread:@selector(speakerUpdated:) withObject:speaker waitUntilDone:NO];
			}
		}
		
		[pool release];
	}
	
	virtual void OnGetVolumeResult(NPT_Result                res,
						   PLT_DeviceDataReference&  device,
						   const char*               channel,
						   NPT_UInt32				 volume,
						   void*                     userdata) {
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
		PPMediaDevice *speaker = (PPMediaDevice *)userdata;
		speaker.deviceVolume = volume;
		CFAbsoluteTime timeNow = CFAbsoluteTimeGetCurrent();
		if ( timeNow - speaker.lastVolChange > 1 ) {
			speaker.volume = speaker.deviceVolume;
			
			[master.delegate performSelectorOnMainThread:@selector(speakerUpdated:) withObject:speaker waitUntilDone:NO];
		}
		
		[pool release];
	}	
	
	PPMediaController *master;
	PLT_CtrlPointReference controlPointRef;
	PLT_MediaController *mediaController;
	PLT_MediaBrowser *mediaBrowser;
};


static PPMediaController *sharedInstance = nil;


@implementation PPMediaController

@synthesize delegate;

+ (PPMediaController *)sharedMediaController {
	if ( !sharedInstance ) {
		sharedInstance = [[PPMediaController alloc] initWithUPnP:[PPUPnP sharedUPnP]];
	}
	return sharedInstance;
}

- (id)initWithUPnP:(PPUPnP *)upnp {
	if ( (self = [super init]) ) {
		mediaController = new PP_MediaController(self, [upnp PLTUPnP]);
		sharedInstance = self;
	}
	return self;
}

- (void)dealloc {

	delete mediaController;
	
	[super dealloc];
}


// Server browsing

- (BOOL)browseContentsOfFolder:(NSString *)folderId onServer:(PPMediaDevice *)server fromIndex:(NSUInteger)start forNumber:(NSUInteger)count userData:(id)userData {
	if ( count == 0 ) {
		count = 30;
	}
	
	NPT_Result result = mediaController->mediaBrowser->Browse([server deviceData]->mediaDevice,
															  [folderId UTF8String], 
															  start,
															  count,
															  false,
															  "dc:date,upnp:genre,res,res@duration,res@size,upnp:albumArtURI,upnp:originalTrackNumber,upnp:album,upnp:artist,upnp:author",
															  "",
															  userData);
	
	return ( result == NPT_SUCCESS );

}


- (BOOL)browseMetadataOfItem:(NSString *)itemId onServer:(PPMediaDevice *)server userData:(id)userData {
	
	NPT_Result result = mediaController->mediaBrowser->Browse([server deviceData]->mediaDevice,
															  [itemId UTF8String], 
															  0,
															  0,
															  true,
															  "dc:date,upnp:genre,res,res@duration,res@size,upnp:albumArtURI,upnp:originalTrackNumber,upnp:album,upnp:artist,upnp:author",
															  "",
															  userData);
	
	return ( result == NPT_SUCCESS );
	
}



// Renderer controlling

- (BOOL)updateMediaInfoForSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->GetMediaInfo(
									[speaker deviceData]->mediaDevice,
									0,
									speaker);
	
	return ( result == NPT_SUCCESS );
}

- (BOOL)updatePositionInfoForSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->GetPositionInfo(
									[speaker deviceData]->mediaDevice,
									0,
									speaker);
	
	return ( result == NPT_SUCCESS );
}

- (BOOL)updateTransportInfoForSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->GetTransportInfo(
									[speaker deviceData]->mediaDevice,
									0,
									speaker);

	return ( result == NPT_SUCCESS );
}


- (BOOL)pauseSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->Pause(
									[speaker deviceData]->mediaDevice,
									0,
									speaker);

	
	return ( result == NPT_SUCCESS );
}

- (BOOL)playSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->Play(
									[speaker deviceData]->mediaDevice,
									0,
									NPT_String("1"),
									speaker);
	

	return ( result == NPT_SUCCESS );
}

- (BOOL)stopSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->Stop(
									[speaker deviceData]->mediaDevice,
									0,
									speaker);
	
	if ( result == NPT_SUCCESS ) {
		speaker.stopRequested = YES;
		speaker.position = 0;
	}
	
	return ( result == NPT_SUCCESS );
}

- (BOOL)setCurrentSong:(PPMediaItem *)song onSpeaker:(PPMediaDevice *)speaker {
	NPT_Cardinal resource_index = 0;
	PLT_MediaObject *track = [song getMediaObject]->mediaObject;
	NPT_Result result = mediaController->mediaController->FindBestResource([speaker deviceData]->mediaDevice, *track, resource_index);
	
	result = mediaController->mediaController->SetAVTransportURI(
							[speaker deviceData]->mediaDevice,
							0,
							(const char*)track->m_Resources[resource_index].m_Uri,
							track->m_Didl,
							speaker);
	
	if ( result == NPT_SUCCESS ) {
		speaker.song = song;
	}

	return ( result == NPT_SUCCESS );
}

- (BOOL)setMuted:(BOOL)mute onSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->SetMute(
								[speaker deviceData]->mediaDevice,
								0,
								"Master",
								mute,
								speaker);
	
	return ( result == NPT_SUCCESS );
}

- (BOOL)updateMutedForSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->GetMute(
							  [speaker deviceData]->mediaDevice,
							  0,
							  "Master",
							  speaker);
	
	return ( result == NPT_SUCCESS );
}


- (BOOL)setVolume:(NSUInteger)volume onSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = NPT_SUCCESS;
	speaker.volume = volume;
	CFAbsoluteTime timeNow = CFAbsoluteTimeGetCurrent();
	if ( timeNow - speaker.lastVolChange > .75 || timeNow < speaker.lastVolChange ) {
		speaker.lastVolChange = timeNow;
		result = mediaController->mediaController->SetVolume(
							  [speaker deviceData]->mediaDevice,
							  0,
							  "Master",
							  volume,
							  speaker);
	}
	
	return ( result == NPT_SUCCESS );
}

- (BOOL)updateVolumeForSpeaker:(PPMediaDevice *)speaker {
	NPT_Result result = mediaController->mediaController->GetVolume(
							  [speaker deviceData]->mediaDevice,
							  0,
							  "Master",
							  speaker);
	
	return ( result == NPT_SUCCESS );	
}

@end
