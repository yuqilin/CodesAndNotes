//
//  SpeakerListController.h
//  CocoaTouchBrowser
//
//  Created by Barry Burton on 12/21/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PPMediaController.h"

@class RootViewController;

@interface SpeakerListController : UITableViewController { }

@property (nonatomic, retain) PPMediaController *controller;
@property (nonatomic, retain) NSMutableArray *list;
@property (nonatomic, retain) UIBarButtonItem *doneButtonItem;
@property (nonatomic, assign) RootViewController *rootVC;

- (id)initWithController:(PPMediaController *)theController andRootViewController:(RootViewController *)theRootVC;
- (void)refreshList;
- (void)addSpeaker:(PPMediaDevice *)speaker;
- (void)removeSpeaker:(PPMediaDevice *)speaker;

@end
