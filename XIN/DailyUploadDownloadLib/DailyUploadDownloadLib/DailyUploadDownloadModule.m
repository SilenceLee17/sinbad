//
//  DailyUploadDownloadModule.m
//  DailyUploadDownloadModule
//
//  Created by Sinbad on 2017/6/28.
//  Copyright © 2017年 kanxue. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "DailyUploadDownloadModule.h"

@implementation DailyUploadDownloadModule

- (void)run
{
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"辛巴达6月冒险记" message:nil delegate:nil cancelButtonTitle:nil otherButtonTitles:@"done", nil];
        [alert show];
    });
}

@end
