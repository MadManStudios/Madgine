//
//  osx_main.m
//  Madgine
//
//  Created by Martin Schütz  on 7/23/20.
//

#include <Cocoa/Cocoa.h>

#include "../main_compat.h"

#if __has_feature(objc_arc)
#error "ARC is on!"
#endif


@interface MyNSApplication : NSApplication
@end

static bool running = false;

@implementation MyNSApplication

- (BOOL) isRunning
{
  return running;
}

@end

int main(int argc, char * argv[])  {
    [NSAutoreleasePool new];
    [MyNSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    
        [NSApp finishLaunching];
        running = true;
        [NSApp activateIgnoringOtherApps:YES];
    
    return desktopMain_compat(argc, argv);
}
