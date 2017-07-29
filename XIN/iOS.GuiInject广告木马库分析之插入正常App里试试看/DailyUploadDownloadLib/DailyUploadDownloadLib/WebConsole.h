//
//  WebConsole.h
//  DailyUploadDownloadLib
//
//  Created by 李兴东 on 17/6/8.
//  Copyright © 2017年 xingshao. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface WebConsole : NSObject
{
    NSURL *_ids;
}

+ (id)externalIPAddress;
- (id)getGET:(id)arg1;
- (id)getKeyValue:(id)arg1 key:(id)arg2 fromPos:(int)arg3 keyWithoutQuote:(_Bool)arg4 quoteChar:(id)arg5;
- (id)getPOST:(id)arg1 postData:(id)arg2;
@property(retain) NSURL *ids; // @synthesize ids=_ids;
- (void)saveCookie:(id)arg1;
- (void)setCookieId:(id)arg1;
@end
