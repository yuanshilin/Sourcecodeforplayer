//
//  ChangePositionView.h
//  SetFilter
//
//  Created by develop on 2024/6/12.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@protocol ChangePositionDelegate <NSObject>

- (void)changePotisonX: (float)xPos Y:(float)yPos channels:(int)count;

@end

@interface ChangePositionView : NSView
@property(assign) id<ChangePositionDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
