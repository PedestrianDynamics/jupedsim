
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import "osx_thread_fix.h"


@interface CocoaMultithreading : NSObject
+ (void)beginMultithreading;
@end

@implementation CocoaMultithreading
+ (void)dummyThread:(id)unused
{
    (void)unused;
}

+ (void)beginMultithreading
{
    [NSThread detachNewThreadSelector:@selector(dummyThread:)
            toTarget:self withObject:nil];
}
@end


void InitMultiThreading()
{
    [[NSThread new] start];
    NSString *msg = @"Starting multithreading";
    NSLog(@"%@", msg);
    //NSLog(@"%@", isMultiThreaded);
    [CocoaMultithreading beginMultithreading];

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    //Do stuff
    [pool release];
}


