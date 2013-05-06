//
//  SpeakerViewController.h
//  CocoaTouchBrowser
//
//  Created by Barry Burton on 12/21/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PPMediaController.h"
#import "PPMediaDevice.h"


@interface SpeakerViewController : UIViewController { }

@property (nonatomic, retain) PPMediaController *controller;
@property (nonatomic, retain) PPMediaDevice *speaker;

@property (nonatomic, retain) IBOutlet UILabel *artistName;
@property (nonatomic, retain) IBOutlet UILabel *albumName;
@property (nonatomic, retain) IBOutlet UILabel *trackName;
@property (nonatomic, retain) IBOutlet UILabel *currentTime;
@property (nonatomic, retain) IBOutlet UILabel *totalTime;

@property (nonatomic, retain) IBOutlet UIButton *playButton;
@property (nonatomic, retain) IBOutlet UIButton *nextButton;


- (id)initWithController:(PPMediaController *)theController speaker:(PPMediaDevice *)theSpeaker song:(PPMediaItem *)theSong;

- (void)speakerUpdated:(PPMediaDevice *)speaker;

- (IBAction)playPressed:(id)sender;
- (IBAction)nextPressed:(id)sender;

@end
