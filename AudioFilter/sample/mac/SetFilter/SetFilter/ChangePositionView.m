//
//  ChangePositionView.m
//  SetFilter
//
//  Created by develop on 2024/6/12.
//

#import "ChangePositionView.h"

@interface ChangePositionView ()
@property(strong) NSTextField* xPos;
@property(strong) NSTextField* yPos;
@property(strong) NSTextField* channles;
@end

@implementation ChangePositionView

- (instancetype)initWithFrame:(NSRect)frameRect
{
    if (self = [super initWithFrame:frameRect]) {
        NSView* selectView = [[NSView alloc] initWithFrame:CGRectMake(0, 0, frameRect.size.width, frameRect.size.height - 100)];
        selectView.wantsLayer = YES;
        selectView.layer.backgroundColor = [NSColor lightGrayColor].CGColor;
        [self addSubview:selectView];
        NSClickGestureRecognizer* click = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(selectViewClicked:)];
        [selectView addGestureRecognizer:click];
        
        float x = 20;
        float y = selectView.frame.origin.y + selectView.frame.size.height + 5;
        self.xPos = [[NSTextField alloc] initWithFrame:CGRectMake(x, y, 40, 20)];
        self.xPos.floatValue = 0.0f;
        [self addSubview:self.xPos];
        
        x = self.xPos.frame.origin.x + self.xPos.frame.size.width + 5;
        self.yPos = [[NSTextField alloc] initWithFrame:CGRectMake(x, y, 40, 20)];
        self.yPos.floatValue = 0.0f;
        [self addSubview:self.yPos];
        
        x = self.yPos.frame.origin.x + self.yPos.frame.size.width + 5;
        self.channles = [[NSTextField alloc] initWithFrame:CGRectMake(x, y, 40, 20)];
        self.channles.intValue = 2;
        [self addSubview:self.channles];
        
        x = self.channles.frame.origin.x + self.channles.frame.size.width + 5;
        NSButton* button = [NSButton buttonWithTitle:@"change" target:self action:@selector(buttonClicked:)];
        button.frame = CGRectMake(x, y, 80, 20);
        [self addSubview:button];

    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)buttonClicked: (id)sender {
    if ([self.delegate respondsToSelector:@selector(changePotisonX:Y:channels:)]) {
        [self.delegate changePotisonX:self.xPos.floatValue Y:self.yPos.floatValue channels:self.channles.intValue];
    }
}

- (void)selectViewClicked: (id)sender {
    NSLog(@"%@", sender);
    NSClickGestureRecognizer* click = (NSClickGestureRecognizer*)sender;
    NSPoint point = [click locationInView:click.view];
    
    NSLog(@"point: %@", NSStringFromPoint(point));
    NSLog(@"area: %@", NSStringFromRect(click.view.frame));
    
    float x = 2 * point.x / click.view.frame.size.width - 1;
    float y = 2 * point.y / click.view.frame.size.height - 1;
    
    float x_i = floorf(x * 10 + 0.5);
    float y_i = floorf(y * 10 + 0.5);
    self.xPos.floatValue = x_i / 10.0f;
    self.yPos.floatValue = y_i / 10.0f;
}
@end
