//
//  SeparateChannelView.h
//  SetFilter
//
//  Created by develop on 2024/4/15.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@protocol AddSepChannelParamDelegate <NSObject>

- (void)sepChannelParamAdded: (NSDictionary*)dict;

@end

@interface SeparateChannelView : NSView

@property(assign) id<AddSepChannelParamDelegate> delegate;
@property(strong) NSMutableDictionary  *sepChannelParams;

@property(strong) NSTextField   *nameText;
@property(strong) NSTextField   *channel1Text;
@property(strong) NSTextField   *channel2Text;
@property(strong) NSTextField   *channel3Text;
@property(strong) NSTextField   *channel4Text;
@property(strong) NSTextField   *channel5Text;
@property(strong) NSTextField   *channel6Text;
@property(strong) NSTextField   *channel7Text;
@property(strong) NSTextField   *channel8Text;
@property(strong) NSTextField   *channel9Text;
@property(strong) NSTextField   *channel10Text;
@property(strong) NSTextField   *channel11Text;
@property(strong) NSTextField   *channel12Text;
@property(strong) NSButton      *addButton;

- (instancetype)initWithFrame:(NSRect)frameRect name: (NSString*)name;

@end

NS_ASSUME_NONNULL_END
