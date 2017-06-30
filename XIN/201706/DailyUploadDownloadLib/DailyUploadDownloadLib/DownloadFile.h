//
//  DownloadFile.h
//  DailyUploadDownloadLib
//
//  Created by 李兴东 on 17/6/8.
//  Copyright © 2017年 xingshao. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DownloadFile : NSObject
{
    int _totalSize;
    NSURLConnection *_myConnection;
}

- (void)connection:(id)arg1 didFailWithError:(id)arg2;
- (void)connection:(id)arg1 didReceiveData:(id)arg2;
- (void)connection:(id)arg1 didReceiveResponse:(id)arg2;
- (void)connectionDidFinishLoading:(id)arg1;
- (void)download:(id)arg1;
@property(retain) NSURLConnection *myConnection; // @synthesize myConnection=_myConnection;
@property int totalSize; // @synthesize totalSize=_totalSize;


@end
