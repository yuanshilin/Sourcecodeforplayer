//
//  CustomSliderView.m
//  SetFilter
//
//  Created by develop on 2024/4/17.
//

#import "CustomSliderView.h"

#define LABEL_WIDTH 40.0f
#define LABEL_HEIGHT 20.0f
#define MARGIN 10.0f

@implementation CustomSliderView

- (instancetype)initWithFrame:(NSRect)frameRect name: (NSString*)name isVertical: (BOOL)isVertical minValue:(double)minValue maxValue:(double)maxValue
{
    if (self = [super initWithFrame:frameRect]) {
        if (isVertical) {
            NSTextField* text = [NSTextField textFieldWithString: name];
            text.editable = NO;
            text.frame = CGRectMake(0, MARGIN, LABEL_WIDTH, LABEL_HEIGHT);
            text.alignment = NSTextAlignmentCenter;
            [self addSubview:text];
            
            NSSlider* slider = [NSSlider sliderWithValue:0 minValue:minValue maxValue:maxValue target:self action:@selector(hzSlider_Changed:)];
//            slider.continuous = NO;
            slider.wantsLayer = YES;
            slider.vertical = true;
            float slider_width = slider.frame.size.height;
            float slider_height = frameRect.size.height - MARGIN*4 - LABEL_HEIGHT * 2;
            slider.frame = CGRectMake((LABEL_WIDTH - slider_width) / 2, MARGIN*2 + LABEL_HEIGHT, slider_width, slider_height);
            
            [self addSubview:slider];
            
            self.gainValue = [NSTextField textFieldWithString:@"0"];
            self.gainValue.editable = NO;
            self.gainValue.frame = CGRectMake(0, slider.frame.origin.y + slider_height + MARGIN, LABEL_WIDTH, LABEL_HEIGHT);
            self.gainValue.alignment = NSTextAlignmentCenter;
            [self addSubview: self.gainValue];
        }
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)hzSlider_Changed:(id)sender {
    NSSlider* slider = sender;
    self.gainValue.intValue = slider.intValue;
}
@end
