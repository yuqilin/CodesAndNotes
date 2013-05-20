//
//  ServerViewController.h
//  CocoaTouchBrowser
//
//  Created by Barry Burton on 12/20/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PPMediaController.h"
#import "PPMediaDevice.h"
#import "PPMediaContainer.h"
#import "RootViewController.h"

@interface ServerViewController : UITableViewController { }

@property (nonatomic, retain) PPMediaController *controller;
@property (nonatomic, retain) PPMediaDevice *server;
@property (nonatomic, retain) PPMediaContainer *container;
@property (nonatomic, retain) NSArray *list;

- (id)initWithController:(PPMediaController *)theController server:(PPMediaDevice *)theServer container:(PPMediaContainer *)theContainer;
- (void)listUpdated;

@end
