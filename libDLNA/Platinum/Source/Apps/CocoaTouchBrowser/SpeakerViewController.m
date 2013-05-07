//
//  SpeakerViewController
//  CocoaTouchBrowser
//
//  Created by Barry Burton on 12/21/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import "SpeakerViewController.h"


@implementation SpeakerViewController

@synthesize controller;
@synthesize speaker;

@synthesize artistName;
@synthesize albumName;
@synthesize trackName;
@synthesize currentTime;
@synthesize totalTime;

@synthesize playButton;
@synthesize nextButton;

// The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
/*
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization.
    }
    return self;
}
*/

- (id)initWithController:(PPMediaController *)theController speaker:(PPMediaDevice *)theSpeaker song:(PPMediaItem *)theSong {
	self = [super initWithNibName:nil bundle:nil];
    if (self) {
        // Custom initialization.
		self.controller = theController;
		self.speaker = theSpeaker;
		if ( theSong ) {
			[self.controller setCurrentSong:theSong onSpeaker:self.speaker];
		}
		[self speakerUpdated:self.speaker];
		[self.controller updateMediaInfoForSpeaker:self.speaker];
		[self.controller updatePositionInfoForSpeaker:self.speaker];
    }
    return self;
}

- (void)speakerUpdated:(PPMediaDevice *)speaker {

	self.trackName.text = self.speaker.song.trackName;
	
	self.albumName.text = self.speaker.song.albumName;
	
	self.artistName.text = self.speaker.song.artistName;
	
	self.currentTime.text = [NSString stringWithFormat:@"%d", self.speaker.position];
	
	self.totalTime.text = [NSString stringWithFormat:@"%d", self.speaker.song.duration];
	
	/*
	 if ( time ) {
	 NSArray *timeComponents = [time componentsSeparatedByString:@":"];
	 int multiplier = 3600 * 1000;
	 int currentTimeNum = 0;
	 for ( NSString *component in timeComponents ) {
	 currentTimeNum += [component doubleValue] > 0 ? (int)([component doubleValue] * (double)multiplier) : 0.0f;
	 multiplier /= 60;
	 }
	 self.position = currentTimeNum;
	 }
	 */
	
	if ( self.speaker.isPlaying ) {
		self.playButton.titleLabel.text = @"Stop";
	} else {
		self.playButton.titleLabel.text = @"Play";
	}
}

- (IBAction)playPressed:(id)sender {
	if ( !self.speaker.isPlaying ) {
		[self.controller playSpeaker:self.speaker];
	} else {
		[self.controller stopSpeaker:self.speaker];
	}
    [self speakerUpdated:self.speaker];
}

- (IBAction)nextPressed:(id)sender {
	
}

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
