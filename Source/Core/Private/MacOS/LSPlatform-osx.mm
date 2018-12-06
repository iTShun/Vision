#include "LSPlatform-osx.h"

#if PLATFORM_OSX

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

/** Application implementation that overrides the terminate logic with custom shutdown, and tracks Esc key presses. */
@interface CocoaApplication : NSApplication
-(void)terminate:(nullable id)sender;
-(void)sendEvent:(NSEvent*)event;
@end


/** Application delegate implementation that activates the application when it finishes launching. */
@interface CocoaAppDelegate : NSObject<NSApplicationDelegate>
@end


/**
 * Contains cursor specific functionality. Encapsulated in objective C so its selectors can be triggered from other
 * threads.
 */
@interface CocoaCursor : NSObject
@property NSCursor* currentCursor;

-(CocoaCursor*) initWithPlatformData:(ls::Platform::Pimpl*)platformData;
-(ls::Vector2I) getPosition;
-(void) setPosition:(const ls::Vector2I&) position;
-(BOOL) clipCursor:(ls::Vector2I&) position;
-(void) updateClipBounds:(NSWindow*) window;
-(void) clipCursorToWindow:(NSValue*) windowValue;
-(void) clipCursorToRect:(NSValue*) rectValue;
-(void) clipCursorDisable;
-(void) setCursor:(NSArray*) params;
-(void) unregisterWindow:(NSWindow*) window;
@end


/** Contains platform specific functionality that is meant to be delayed executed from the sim thread, through Platform. */
@interface CocoaFPlatform : NSObject
-(CocoaFPlatform*) initWithPlatformData:(ls::Platform::Pimpl*)platformData;
-(void) setCaptionNonClientAreas:(NSArray*) params;
-(void) resetNonClientAreas:(NSValue*) windowIdValue;
-(void) openFolder:(NSURL*) url;
-(void) setClipboardText:(NSString*) text;
-(NSString*) getClipboardText;
-(int32_t) getClipboardChangeCount;
@end

namespace ls
{
    class CocoaWindow;
    
    /** Returns the name of the current application based on the information in the app. bundle. */
    static NSString* getAppName();
    
    /** Creates the default menu for the application menu bar. */
    static void createApplicationMenu();
    
    /** Contains information about a modal window session. */
    struct ModalWindowInfo
    {
        UINT32 windowId;
        NSModalSession session;
    };
    
    struct Platform::Pimpl
    {
        CocoaAppDelegate* appDelegate = nil;
        
        Mutex windowMutex;
        CocoaWindow* mainWindow = nullptr;
        UnorderedMap<UINT32, CocoaWindow*> allWindows;
        Vector<ModalWindowInfo> modalWindows;
        
        CocoaFPlatform* platformManager = nil;
        
        // Cursor
        CocoaCursor* cursorManager = nil;
        
        Mutex cursorMutex;
        bool cursorIsHidden = false;
        Vector2I cursorPos;
        
        // Clipboard
        Mutex clipboardMutex;
        String cachedClipboardData;
        INT32 clipboardChangeCount = -1;
    };
    
    Platform::Pimpl* Platform::mData = ls_new<Platform::Pimpl>();
    
    Platform::~Platform()
    {
        ls_delete(mData);
        mData = nullptr;
    }
    
    void Platform::_messagePump()
    { @autoreleasepool {
        while(true)
        {
            if(!mData->modalWindows.empty())
            {
                NSModalSession session = mData->modalWindows.back().session;
                [NSApp runModalSession:session];
                break;
            }
            else
            {
                NSEvent* event = [NSApp
                                  nextEventMatchingMask:NSEventMaskAny
                                  untilDate:[NSDate distantPast]
                                  inMode:NSDefaultRunLoopMode
                                  dequeue:YES];
                
                if (!event)
                    break;
                
                [NSApp sendEvent:event];
            }
        }
    }}
    
    void Platform::_startUp()
    {
        mData->appDelegate = [[CocoaAppDelegate alloc] init];
        mData->cursorManager = [[CocoaCursor alloc] initWithPlatformData:mData];
        mData->platformManager = [[CocoaFPlatform alloc] initWithPlatformData:mData];
        [CocoaApplication sharedApplication];
        
        [NSApp setDelegate:mData->appDelegate];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        createApplicationMenu();
        
        [NSApp finishLaunching];
    }
    
    void Platform::_update()
    {
        {
            Lock lock(mData->cursorMutex);
            mData->cursorPos = [mData->cursorManager getPosition];
        }
        
        INT32 changeCount = [mData->platformManager getClipboardChangeCount];
        if(mData->clipboardChangeCount != changeCount)
        {
            NSString* string = [mData->platformManager getClipboardText];
            String utf8String;
            
            if(string)
                utf8String = [string UTF8String];
            
            {
                Lock lock(mData->clipboardMutex);
                mData->cachedClipboardData = utf8String;
            }
            
            mData->clipboardChangeCount = changeCount;
        }
        
        _messagePump();
    }
    
    void Platform::_coreUpdate()
    {
        
    }
    
    void Platform::_shutDown()
    {
        
    }
    
    NSString* getAppName()
    {
        NSString* appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"];
        if (!appName)
            appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
        
        if (![appName length]) {
            appName = [[NSProcessInfo processInfo] processName];
        }
        
        return appName;
    }
    
    void createApplicationMenu()
    { @autoreleasepool {
        NSMenu* mainMenu = [[NSMenu alloc] init];
        [NSApp setMainMenu:mainMenu];
        
        NSString* appName = getAppName();
        NSMenu* appleMenu = [[NSMenu alloc] initWithTitle:@""];
        
        NSString* aboutTitle = [@"About " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:aboutTitle
                             action:@selector(orderFrontStandardAboutPanel:)
                      keyEquivalent:@""];
        
        [appleMenu addItem:[NSMenuItem separatorItem]];
        
        NSString* hideTitle = [@"Hide " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:hideTitle action:@selector(hide:) keyEquivalent:@"h"];
        
        NSMenuItem* hideOthersMenuItem = [appleMenu
                                          addItemWithTitle:@"Hide Others"
                                          action:@selector(hideOtherApplications:)
                                          keyEquivalent:@"h"];
        
        [hideOthersMenuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];
        
        [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
        
        [appleMenu addItem:[NSMenuItem separatorItem]];
        
        NSString* quitTitle = [@"Quit " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
        
        NSMenuItem* appleMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
        [appleMenuItem setSubmenu:appleMenu];
        [[NSApp mainMenu] addItem:appleMenuItem];
    }}
    
    void flipY(NSScreen* screen, NSRect& rect)
    {
        NSRect screenFrame = [screen frame];
        
        rect.origin.y = screenFrame.size.height - (rect.origin.y + rect.size.height);
    }
    
    void flipY(NSScreen* screen, NSPoint &point)
    {
        NSRect screenFrame = [screen frame];
        
        point.y = screenFrame.size.height - point.y;
    }
    
    void flipYWindow(NSWindow* window, NSPoint &point)
    {
        NSRect windowFrame = [window frame];
        
        point.y = windowFrame.size.height - point.y;
    }
}


///////////////////////////////////////////////////
@implementation CocoaApplication
-(void)terminate:(nullable id)sender
{
//    ls::gCoreApplication().quitRequested();
}

-(void)sendEvent:(NSEvent *)event
{
    // Handle Esc & Tab key here, as it doesn't seem to be reported elsewhere
    if([event type] == NSEventTypeKeyDown)
    {
        if([event keyCode] == 53) // Escape key
        {
//            ls::InputCommandType ic = ls::InputCommandType::Escape;
//            ls::MacOSPlatform::sendInputCommandEvent(ic);
        }
        else if([event keyCode] == 48) // Tab key
        {
//            ls::InputCommandType ic = ls::InputCommandType::Tab;
//            ls::MacOSPlatform::sendInputCommandEvent(ic);
        }
    }
    
    [super sendEvent:event];
}
@end


///////////////////////////////////////////////////
@implementation CocoaAppDelegate : NSObject
-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [NSApp activateIgnoringOtherApps:YES];
}
@end


///////////////////////////////////////////////////
@implementation CocoaCursor
{
    ls::Platform::Pimpl* platformData;
    
    bool cursorClipEnabled;
    ls::Rect2I cursorClipRect;
    NSWindow* cursorClipWindow;
}

- (CocoaCursor*)initWithPlatformData:(ls::Platform::Pimpl*)data
{
    self = [super init];
    
    platformData = data;
    return self;
}

- (ls::Vector2I)getPosition
{
    NSPoint point = [NSEvent mouseLocation];
    
    for (NSScreen* screen in [NSScreen screens])
    {
        NSRect frame = [screen frame];
        if (NSMouseInRect(point, frame, NO))
            ls::flipY(screen, point);
    }
    
    ls::Vector2I output;
    output.x = (int32_t)point.x;
    output.y = (int32_t)point.y;
    
    return output;
}

- (void)setPosition:(const ls::Vector2I&)position
{
    NSPoint point = NSMakePoint(position.x, position.y);
    CGWarpMouseCursorPosition(point);
    
    Lock lock(platformData->cursorMutex);
    platformData->cursorPos = position;
}

- (BOOL)clipCursor:(ls::Vector2I&)position
{
    if(!cursorClipEnabled)
        return false;
    
    int32_t clippedX = position.x - cursorClipRect.x;
    int32_t clippedY = position.y - cursorClipRect.y;
    
    if(clippedX < 0)
        clippedX = 0;
        else if(clippedX >= (int32_t)cursorClipRect.width)
            clippedX = cursorClipRect.width > 0 ? cursorClipRect.width - 1 : 0;
            
            if(clippedY < 0)
                clippedY = 0;
                else if(clippedY >= (int32_t)cursorClipRect.height)
                    clippedY = cursorClipRect.height > 0 ? cursorClipRect.height - 1 : 0;
                    
                    clippedX += cursorClipRect.x;
                    clippedY += cursorClipRect.y;
                    
                    if(clippedX != position.x || clippedY != position.y)
                    {
                        position.x = clippedX;
                        position.y = clippedY;
                        
                        return true;
                    }
    
    return false;
}

- (void)updateClipBounds:(NSWindow*)window
{
    if(!cursorClipEnabled || cursorClipWindow != window)
        return;
    
    NSRect rect = [window contentRectForFrameRect:[window frame]];
    ls::flipY([window screen], rect);
    
    cursorClipRect.x = (int32_t)rect.origin.x;
    cursorClipRect.y = (int32_t)rect.origin.y;
    cursorClipRect.width = (uint32_t)rect.size.width;
    cursorClipRect.height = (uint32_t)rect.size.height;
}

- (void)clipCursorToWindow:(NSValue*)windowValue
{
    ls::CocoaWindow* window;
    [windowValue getValue:&window];
    
    cursorClipEnabled = true;
//    cursorClipWindow = window->_getPrivateData()->window; Todo
    
    [self updateClipBounds:cursorClipWindow];
    
    ls::Vector2I pos = [self getPosition];
    
    if([self clipCursor:pos])
        [self setPosition:pos];
}

- (void)clipCursorToRect:(NSValue*)rectValue
{
    ls::Rect2I rect;
    [rectValue getValue:&rect];
    
    cursorClipEnabled = true;
    cursorClipRect = rect;
    cursorClipWindow = nullptr;
    
    ls::Vector2I pos = [self getPosition];
    
    if([self clipCursor:pos])
        [self setPosition:pos];
}

- (void)clipCursorDisable
{
    cursorClipEnabled = false;
    cursorClipWindow = nullptr;
}

- (void)setCursor:(NSArray*)params
{
    NSCursor* cursor = params[0];
    NSValue* hotSpotValue = params[1];
    
    NSPoint hotSpot;
    [hotSpotValue getValue:&hotSpot];
    
    [self setCurrentCursor:cursor];
    
    for(auto& entry : platformData->allWindows)
    {
        NSWindow* window = nil;// entry.second->_getPrivateData()->window;Todo
        [window invalidateCursorRectsForView:[window contentView]];
    }
}

- (void)unregisterWindow:(NSWindow*)window
{
    if(cursorClipEnabled && cursorClipWindow == window)
        [self clipCursorDisable];
}

@end


///////////////////////////////////////////////////
@implementation CocoaFPlatform
{
    ls::Platform::Pimpl* mPlatformData;
}

- (CocoaFPlatform*)initWithPlatformData:(ls::Platform::Pimpl*)platformData
{
    self = [super init];
    
    mPlatformData = platformData;
    return self;
}

- (void)setCaptionNonClientAreas:(NSArray*)params
{
    NSValue* windowIdValue = params[0];
    
    ls::UINT32 windowId;
    [windowIdValue getValue:&windowId];
    
    auto iterFind = mPlatformData->allWindows.find(windowId);
    if(iterFind == mPlatformData->allWindows.end())
        return;
    
    ls::CocoaWindow* window = iterFind->second;
    
    NSUInteger numEntries = [params count] - 1;
    
    ls::Vector<ls::Rect2I> areas;
    for(NSUInteger i = 0; i < numEntries; i++)
    {
        NSValue* value = params[i + 1];
        
        ls::Rect2I area;
        [value getValue:&area];
        
        areas.push_back(area);
    }
    
//    window->_setDragZones(areas);Todo
}

- (void)resetNonClientAreas:(NSValue*) windowIdValue
{
    ls::UINT32 windowId;
    [windowIdValue getValue:&windowId];
    
    auto iterFind = mPlatformData->allWindows.find(windowId);
    if(iterFind == mPlatformData->allWindows.end())
        return;
    
    ls::CocoaWindow* window = iterFind->second;
//    window->_setDragZones({});Todo
}

- (void)openFolder:(NSURL*)url
{
    [[NSWorkspace sharedWorkspace] openURL:url];
}

- (void) setClipboardText:(NSString*) text
{ @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    NSArray* objects = [NSArray arrayWithObject:text];
    [pasteboard writeObjects:objects];
}}

- (NSString*) getClipboardText
{ @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    NSArray* classes = [NSArray arrayWithObjects:[NSString class], nil];
    NSDictionary* options = [NSDictionary dictionary];
    
    NSArray* items = [pasteboard readObjectsForClasses:classes options:options];
    if(!items || items.count == 0)
        return nil;
    
    return (NSString*) items[0];
}}

- (int32_t)getClipboardChangeCount
{
    return (int32_t)[[NSPasteboard generalPasteboard] changeCount];
}

@end

#endif // PLATFORM_OSX
