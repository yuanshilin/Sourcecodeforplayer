//
//  FilterParamView.m
//  SetFilter
//
//  Created by develop on 2024/3/11.
//

#import "FilterParamView.h"

@implementation FilterParamView

- (instancetype)initWithFrame:(NSRect)frameRect
{
    if (self = [super initWithFrame:frameRect]) {
        self.typeText = [[NSTextField alloc] initWithFrame:CGRectMake(0, 0, 30, 21)];
        [self addSubview:self.typeText];
        
        self.freqText = [[NSTextField alloc] initWithFrame:CGRectMake(30, 0, 60, 21)];
        [self addSubview:self.freqText];
        
        self.qText = [[NSTextField alloc] initWithFrame:CGRectMake(90, 0, 60, 21)];
        [self addSubview:self.qText];

        self.channelsText = [[NSTextField alloc] initWithFrame:CGRectMake(150, 0, 60, 21)];
        [self addSubview:self.channelsText];
        
        self.gainText = [[NSTextField alloc] initWithFrame:CGRectMake(210, 0, 60, 21)];
        [self addSubview:self.gainText];
        
        self.addButton = [NSButton buttonWithTitle:@"addFilter" target:self action:@selector(addClicked:)];
        self.addButton.frame = CGRectMake(270, 0, 80, 21);
        [self addSubview:self.addButton];
        
        self.oneFilterParams = [NSMutableDictionary dictionaryWithCapacity:16];
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)addClicked: (id)sender
{
    NSLog(@"add clicked");
    
    if (self.typeText.intValue <= 0 || self.typeText.intValue > 11) {
        NSLog(@"error type");
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"error type"];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    } else if (self.freqText.intValue <= 0 && self.typeText.intValue != 1) {
        NSLog(@"error freq");
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"error freq"];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    } else if (self.qText.floatValue < 0 && self.typeText.intValue != 1) {
        NSLog(@"error q factor");
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"error q factor"];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    } else if (self.channelsText.intValue < 0) {
        NSLog(@"error channels");
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"error channels"];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    }
    [self.oneFilterParams setValue:[NSNumber numberWithInt:self.typeText.intValue] forKey:@"type"];
    [self.oneFilterParams setValue:[NSNumber numberWithInt:self.channelsText.intValue] forKey:@"channels"];

    if (self.typeText.intValue != 1) {
        [self.oneFilterParams setValue:[NSNumber numberWithInt:self.freqText.intValue] forKey:@"freq"];
        [self.oneFilterParams setValue:[NSNumber numberWithFloat:self.qText.floatValue] forKey:@"Qfactor"];
    }
    int type = self.typeText.intValue;
    if (type == 8 || type == 10 || type == 11 || type == 1)
        [self.oneFilterParams setValue:[NSNumber numberWithInt:self.gainText.floatValue] forKey:@"gain"];

    
    if ([self.delegate respondsToSelector:@selector(filterParamAdded:)]) {
        [self.delegate filterParamAdded:self.oneFilterParams];
        self.typeText.editable = self.freqText.editable = false;
    }
}
@end
