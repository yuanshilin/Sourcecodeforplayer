//
//  CustomSliderView.h
//  SetFilter
//
//  Created by develop on 2024/4/17.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface CustomSliderView : NSView

@property (strong) NSTextField* gainValue;
@property (assign) NSInteger    freq;

- (instancetype)initWithFrame:(NSRect)frameRect name: (NSString*)name isVertical: (BOOL)isVertical minValue:(double)minValue maxValue:(double)maxValue;

@end

NS_ASSUME_NONNULL_END
