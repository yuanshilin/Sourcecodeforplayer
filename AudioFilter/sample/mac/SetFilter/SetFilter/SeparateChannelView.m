//
//  SeparateChannelView.m
//  SetFilter
//
//  Created by develop on 2024/4/15.
//

#import "SeparateChannelView.h"

#define TEXT_WIDTH 40.0f

@implementation SeparateChannelView

- (instancetype)initWithFrame:(NSRect)frameRect name: (NSString*)name
{
    if (self = [super initWithFrame:frameRect]) {
        bool b = [name isEqualToString:@""];

        
        for (int i = 0; i < 12; i++) {
            NSTextField* text = [[NSTextField alloc] initWithFrame:CGRectMake(60 + i * TEXT_WIDTH, 0, TEXT_WIDTH, 21)];
            [self addSubview:text];
            text.tag = i + 1;
            text.alignment = NSTextAlignmentCenter;
            if (b) {
                text.stringValue = [NSString stringWithFormat:@"%ld", text.tag];
                text.editable = NO;
            } else
                text.backgroundColor = [NSColor colorWithRed:0.3 green:0.3 blue:0.3 alpha:0.3];

        }


        if (!b) {
            self.nameText = [[NSTextField alloc] initWithFrame:CGRectMake(0, 0, 60, 21)];
            self.nameText.stringValue = name;
            self.nameText.alignment = NSTextAlignmentCenter;
            self.nameText.editable = NO;
            [self addSubview:self.nameText];

            self.addButton = [NSButton buttonWithTitle:@"addFilter" target:self action:@selector(addClicked:)];
            self.addButton.frame = CGRectMake(60 + TEXT_WIDTH * 12, 0, 80, 21);
            [self addSubview:self.addButton];
            
            self.sepChannelParams = [NSMutableDictionary dictionaryWithCapacity:16];
        }
        
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)addClicked: (id)sender {
    if ([self.nameText.stringValue isEqualToString:@"gain"]) {
        [self.sepChannelParams setValue:[NSNumber numberWithInt:1] forKey:@"type"];

    } else if ([self.nameText.stringValue isEqualToString:@"delay"]) {
        [self.sepChannelParams setValue:[NSNumber numberWithInt:12] forKey:@"type"];
        for (NSView* view in self.subviews) {
            if ([[view class] isEqualTo:[NSTextField class]]) {
                NSTextField* textfield = (NSTextField*)view;
                float value = textfield.floatValue;
                if (value > 200.0f) {
                    value = 200.0f;
                }
                if (value > 0) {
                    NSString* key = [NSString stringWithFormat:@"channel%ld", textfield.tag];
                    [self.sepChannelParams setValue:[NSNumber numberWithFloat:value] forKey:key];
                }
            }
        }
    }
    if ([self.delegate respondsToSelector:@selector(sepChannelParamAdded:)]) {
        [self.delegate sepChannelParamAdded:self.sepChannelParams];
    }
}
@end
