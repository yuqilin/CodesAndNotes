//
//  PP_MediaObject.h
//  Platinum
//
//  Created by Barry Burton on 12/21/10.
//  Copyright 2010 Gravity Mobile. All rights reserved.
//

#ifndef _PP_MEDIA_OBJECT_H_
#define _PP_MEDIA_OBJECT_H_

#import "PltMediaItem.h"
#import "PltMediaBrowser.h"

class PP_MediaObject {
public:
	PP_MediaObject(PLT_MediaObject *object)
		: mediaObject(object)
		, owner(0)
		, childList()
	{
		
	}
	
	virtual ~PP_MediaObject() {
		
	}
	
	
	PLT_MediaObject *mediaObject;
	id owner;
	NPT_List<PLT_MediaObjectListReference> childList;
};

#endif /* _PP_MEDIA_OBJECT_H_ */
