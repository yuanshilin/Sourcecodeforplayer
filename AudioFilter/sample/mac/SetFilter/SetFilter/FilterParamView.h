//
//  FilterParamView.h
//  SetFilter
//
//  Created by develop on 2024/3/11.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@protocol AddFilterParamDelegate <NSObject>

- (void)filterParamAdded: (NSDictionary*)dict;

@end

@interface FilterParamView : NSView

@property(assign) id<AddFilterParamDelegate> delegate;
@property(strong) NSMutableDictionary  *oneFilterParams;

@property(strong) NSTextField   *typeText;
@property(strong) NSTextField   *freqText;
@property(strong) NSTextField   *qText;
@property(strong) NSTextField   *channelsText;
@property(strong) NSTextField   *gainText;
@property(strong) NSButton      *addButton;

- (instancetype)initWithFrame:(NSRect)frameRect;
@end

NS_ASSUME_NONNULL_END
