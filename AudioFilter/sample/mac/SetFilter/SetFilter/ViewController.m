//
//  ViewController.m
//  SetFilter
//
//  Created by develop on 2024/3/5.
//

#import "ViewController.h"
#import "FilterParamView.h"
#import "SeparateChannelView.h"
#import "CustomSliderView.h"
#import "ChangePositionView.h"

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_GAIN 20
#define MIN_GAIN -20
#define DEFAULT_Q 1.0f / sqrt(2.0f)

#define LABEL_WIDTH 40.0f
#define LABEL_HEIGHT 20.0f
#define MARGIN 10.0f

#define MAX_CHANGED_GAIN 6.0f

#define Table_Width 330.0f

static int freqs[] = {20,25,31,40,50,63,80,100,118, 156,196,264,326, 408,524,641,800,998,1322,1579, 2000,2477,3150, 4000, 5000, 6300,8000, 10000, 12500, 16000, 20000};

@interface ViewController ()<NSTextViewDelegate, AddFilterParamDelegate, AddSepChannelParamDelegate, ChangePositionDelegate>

@property (strong) NSMutableDictionary* jsonDict;
@property (strong) NSMutableDictionary* filterParams;

@property (strong) NSString* jsonString;
@property (strong) IBOutlet NSTextView *jsonText;
@property (strong) IBOutlet NSView *hzSliderView;
@property (strong) IBOutlet NSView *sepChannelView;
@property (strong) IBOutlet NSView *changePositionView;

@property (strong) IBOutlet NSTextField *ipText;
@property (strong) IBOutlet NSTextField *portText;
@property (strong) IBOutlet NSTextField *sliderQfactor;

@property (strong) IBOutlet NSTextField *typeText;
@property (weak) NSView* targetView;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self setupSliders];
    [self setupSepChannelView];
    [self setupChangePositionView];

    self.jsonDict = [NSMutableDictionary dictionaryWithCapacity:16];
    self.filterParams = [NSMutableDictionary dictionaryWithCapacity:16];
    self.jsonText.editable = YES;
    self.jsonText.delegate = self;
    self.jsonString = self.jsonText.string;
    // Do any additional setup after loading the view.
    
    self.targetView = self.typeText;
    self.sliderQfactor.floatValue = DEFAULT_Q;
}

- (void)setupChangePositionView {
    ChangePositionView* view = [[ChangePositionView alloc] initWithFrame:CGRectMake(0, 0, 256, 384)];
    view.delegate = self;
    
    [self.changePositionView addSubview:view];
}

- (void)setupSepChannelView {

    SeparateChannelView* view1 = [[SeparateChannelView alloc] initWithFrame:CGRectMake(0, 64, 620, 32) name:@""];
    [self.sepChannelView addSubview:view1];
    
    SeparateChannelView* view3 = [[SeparateChannelView alloc] initWithFrame:CGRectMake(0, 32, 620, 32) name:@"delay"];
    view3.delegate = self;
    [self.sepChannelView addSubview:view3];

//    SeparateChannelView* view2 = [[SeparateChannelView alloc] initWithFrame:CGRectMake(0, 0, 560, 32) name:@"gain"];
//    [self.sepChannelView addSubview:view2];


}

- (void)setupSliders {
//    NSLog(@"view size %@", NSStringFromSize(self.hzSliderView.frame.size));
    self.hzSliderView.wantsLayer = YES;
    self.hzSliderView.layer.backgroundColor = [NSColor lightGrayColor].CGColor;
    
//    float slider_height = self.hzSliderView.frame.size.height - MARGIN*3 - LABEL_HEIGHT * 2;
//    float slider_center_y = MARGIN + slider_height / 2;
    int slider_count = sizeof(freqs) / sizeof(int);
    NSLog(@"count %d", slider_count);
    float slider_offset = self.hzSliderView.frame.size.width / (slider_count + 1);

    for (int i = 0; i < slider_count; i++) {
        
        float slider_center_x = slider_offset * (i + 1);
        
        CustomSliderView* view = [[CustomSliderView alloc] initWithFrame:CGRectMake(slider_center_x - LABEL_WIDTH / 2, 0, LABEL_WIDTH, self.hzSliderView.frame.size.height) name:[self freqToString:freqs[i]] isVertical:YES minValue:MIN_GAIN maxValue:MAX_GAIN];
        view.freq = freqs[i];
        [self.hzSliderView addSubview:view];
        
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (void)refreshTextView {
    if ([self.filterParams count] > 0)  {
        
        NSMutableArray *filters = [NSMutableArray arrayWithCapacity:16];

        for (NSDictionary* dict in self.filterParams.allValues) {
            [filters addObject:dict];
        }
        
        [self.jsonDict setValue:filters forKey:@"filters"];
        self.jsonString = [self dictionaryToJson:self.jsonDict];
        NSLog(@"json: %@", self.jsonString);
        self.jsonText.string = self.jsonString;

    }
}

#pragma mark -- Delegate --
- (void)textDidChange:(NSNotification *)notification
{
    NSTextView* textView = notification.object;
//    NSLog(@"[%s], %@", __FUNCTION__, textView.string);
    
    self.jsonString = textView.string;
}

- (void)filterParamAdded:(NSDictionary *)dict
{
    int type = [[dict valueForKey:@"type"] intValue];
    int freq = [[dict valueForKey:@"freq"] intValue];
    int channel = [[dict valueForKey:@"channels"] intValue];
    NSDictionary* myDict = [NSDictionary dictionaryWithDictionary:dict];
    [self.filterParams setValue:myDict forKey:[NSString stringWithFormat:@"%d/%d/%d", type, freq, channel]];

    [self refreshTextView];
}

- (void)sepChannelParamAdded:(NSDictionary *)dict
{
    int type = [[dict valueForKey:@"type"] intValue];
    int freq = [[dict valueForKey:@"freq"] intValue];
    int channel = [[dict valueForKey:@"channels"] intValue];
    NSDictionary* myDict = [NSDictionary dictionaryWithDictionary:dict];
    [self.filterParams setValue:myDict forKey:[NSString stringWithFormat:@"%d/%d/%d", type, freq, channel]];

    [self refreshTextView];

}

- (void)changePotisonX:(float)xPos Y:(float)yPos channels:(int)count {
    
    if (xPos > 1.0f) xPos = 1.0f;
    if (xPos < -1.0f) xPos = -1.0f;
    if (yPos > 1.0f) yPos = 1.0f;
    if (yPos < -1.0f) yPos = -1.0f;
    
    NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:10];
    if (count == 2) {
        int channels = 0x01;
        float gain = -xPos * MAX_CHANGED_GAIN;
        [dict setValue:@(1) forKey:@"type"];
        [dict setValue:@(channels) forKey:@"channels"];
        [dict setValue:@(gain) forKey:@"gain"];
        [self.filterParams setValue:[NSDictionary dictionaryWithDictionary:dict] forKey:[NSString stringWithFormat:@"1/%d/gain", channels]];
        [dict removeAllObjects];
        channels = 0x02;
        gain = xPos * MAX_CHANGED_GAIN;
        [dict setValue:@(1) forKey:@"type"];
        [dict setValue:@(channels) forKey:@"channels"];
        [dict setValue:@(gain) forKey:@"gain"];
        [self.filterParams setValue:[NSDictionary dictionaryWithDictionary:dict] forKey:[NSString stringWithFormat:@"1/%d/gain", channels]];

    } else if (count == 10) {
        int channels = 0x01 | 0x40;
        float gain = -xPos * MAX_CHANGED_GAIN + yPos * MAX_CHANGED_GAIN;
        [dict setValue:@(1) forKey:@"type"];
        [dict setValue:@(channels) forKey:@"channels"];
        [dict setValue:@(gain) forKey:@"gain"];
        [self.filterParams setValue:[NSDictionary dictionaryWithDictionary:dict] forKey:[NSString stringWithFormat:@"1/%d/gain", channels]];
        [dict removeAllObjects];
        channels = 0x02 | 0x80;
        gain = xPos * MAX_CHANGED_GAIN + yPos * MAX_CHANGED_GAIN;
        [dict setValue:@(1) forKey:@"type"];
        [dict setValue:@(channels) forKey:@"channels"];
        [dict setValue:@(gain) forKey:@"gain"];
        [self.filterParams setValue:[NSDictionary dictionaryWithDictionary:dict] forKey:[NSString stringWithFormat:@"1/%d/gain", channels]];
        [dict removeAllObjects];
        channels = 0x04;
        gain = yPos * MAX_CHANGED_GAIN;
        [dict setValue:@(1) forKey:@"type"];
        [dict setValue:@(channels) forKey:@"channels"];
        [dict setValue:@(gain) forKey:@"gain"];
        [self.filterParams setValue:[NSDictionary dictionaryWithDictionary:dict] forKey:[NSString stringWithFormat:@"1/%d/gain", channels]];
        [dict removeAllObjects];
        channels = 0x10 | 0x100;
        gain = -xPos * MAX_CHANGED_GAIN - yPos * MAX_CHANGED_GAIN;
        [dict setValue:@(1) forKey:@"type"];
        [dict setValue:@(channels) forKey:@"channels"];
        [dict setValue:@(gain) forKey:@"gain"];
        [self.filterParams setValue:[NSDictionary dictionaryWithDictionary:dict] forKey:[NSString stringWithFormat:@"1/%d/gain", channels]];
        [dict removeAllObjects];
        channels = 0x20 | 0x200;
        gain = xPos * MAX_CHANGED_GAIN - yPos * MAX_CHANGED_GAIN;
        [dict setValue:@(1) forKey:@"type"];
        [dict setValue:@(channels) forKey:@"channels"];
        [dict setValue:@(gain) forKey:@"gain"];
        [self.filterParams setValue:[NSDictionary dictionaryWithDictionary:dict] forKey:[NSString stringWithFormat:@"1/%d/gain", channels]];
    } else if (count == 12) {
        
    }

    [self refreshTextView];
}

#pragma mark -- Button Action --
- (IBAction)addButtonClied:(id)sender {
    FilterParamView* view = [[FilterParamView alloc] initWithFrame:CGRectMake(5, self.targetView.frame.origin.y + 25 , Table_Width, 21.0f)];
    view.delegate = self;
    [self.view addSubview:view];
    self.targetView = view;
}

- (IBAction)openButtonClicked:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    
    [panel setAllowsMultipleSelection:NO];  //是否允许多选file
    
    [panel beginWithCompletionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK) {
            for (NSURL* elemnet in [panel URLs]) {
                NSError* error = nil;
                self.jsonString = [NSString stringWithContentsOfURL:elemnet encoding:NSUTF8StringEncoding error:&error];
                NSLog(@"content: %@", self.jsonString);
                self.jsonText.string = self.jsonString;
            }
        }
    }];
}

- (IBAction)saveButtonClicked:(id)sender {
    NSSavePanel *panel = [NSSavePanel savePanel];
    panel.title = @"保存文件";
    [panel setMessage:@"选择文件保存地址"];//提示文字
    
    [panel setDirectoryURL:[NSURL fileURLWithPath:[NSHomeDirectory() stringByAppendingPathComponent:@"test"]]];//设置默认打开路径
    
    [panel setNameFieldStringValue:@"filter"];
    [panel setAllowsOtherFileTypes:YES];
    [panel setAllowedFileTypes:@[@"json"]];
    [panel setExtensionHidden:NO];
    [panel setCanCreateDirectories:YES];

    [panel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result){
        if (result == NSModalResponseOK)
        {
            NSString *path = [[panel URL] path];
            NSData *jsonData = [NSData dataWithBytes:[self.jsonString UTF8String] length:self.jsonString.length];
            [jsonData writeToFile:path atomically:YES];
        }
    }];
}

- (IBAction)resetButtonClicked:(id)sender {
    for (NSView* view in self.view.subviews) {
        if ([[view class] isEqualTo:[FilterParamView class]]) {
            [view removeFromSuperview];
        }
    }
    for (NSView* view in self.hzSliderView.subviews) {
        if ([[view class] isEqualTo:[NSSlider class]]) {
            NSSlider* slider = (NSSlider*)view;
            slider.floatValue = 0.0f;
        }
    }
    self.jsonString = self.jsonText.string = @"{ \"filters\" : []}";
    [self.jsonDict removeAllObjects];
    [self.filterParams removeAllObjects];
    self.targetView = self.typeText;
}

- (IBAction)sendButtonClicked:(id)sender {
    if ([self isJsonString:self.jsonString]) {
        NSLog(@"is json");
        
        [self sendToServer:self.jsonString];
    } else {
        NSLog(@"is not json");
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"需要json格式的字符串"];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    }
}

- (IBAction)sliderAddToFilterButtonClicked:(id)sender {
    for (NSView* view in self.hzSliderView.subviews) {
        if ([[view class] isEqualTo:[CustomSliderView class]]) {
            CustomSliderView* slider = (CustomSliderView*)view;
            float gain = slider.gainValue.floatValue;
            if (gain != 0) {
                NSDictionary* dict = @{@"type":@(8),@"freq":@(slider.freq),@"Qfactor":@(self.sliderQfactor.floatValue),@"gain":@(gain),@"channels":@(0xFFFF)};
                [self.filterParams setValue:dict forKey:[NSString stringWithFormat:@"8/%ld/65535", slider.freq]];
            }
        }
    }
    [self refreshTextView];
}



#pragma mark -- Internal Function --
- (NSString*)dictionaryToJson:(NSDictionary *)dic {
    NSError *parseError = nil;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dic options:NSJSONWritingPrettyPrinted error:&parseError];
    return [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
}

- (Boolean)isJsonString: (NSString*)string {
    NSData *jsonData =  [self.jsonString dataUsingEncoding:NSUTF8StringEncoding];
    NSDictionary *jsonObj = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers error:nil];
    
    NSLog(@"convertJsonToOC = %@" ,jsonObj);
    
    return jsonObj != nil;
}

- (void)sendToServer: (NSString*)string {
    
    NSLog(@"ip: %@, port: %@, string length: %lu", self.ipText.stringValue, self.portText.stringValue, string.length);
    struct sockaddr_in serverAdd;
    
    bzero(&serverAdd, sizeof(serverAdd));
    serverAdd.sin_family = AF_INET;
    serverAdd.sin_addr.s_addr = inet_addr([self.ipText.stringValue UTF8String]);
    serverAdd.sin_port = htons(self.portText.intValue);
    
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int connResult = connect(connfd, (struct sockaddr *)&serverAdd, sizeof(serverAdd));
    if (connResult < 0) {
        int err = errno;
        printf("连接失败，errno ＝ %d\n", err);
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"连接失败，errno ＝ %d", err];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        
        close(connfd);

        return;
    }
    else
    {
        printf("连接成功\n");
    }
    
    ssize_t writeLen;
     
    int count = 0;
    while (1)
    {
        count++;
        if (count == 2) {
            exit(0);
        }
        NSUInteger bufferLength = 4 + [string length];
        uint8_t* buffer = (uint8_t*)malloc(bufferLength);
        buffer[0] = 0x24;
        buffer[1] = 0;
        buffer[2] = ([string length] >> 8) & 0xFF;
        buffer[3] = [string length] & 0xFF;
        memcpy(buffer + 4, [string UTF8String], [string length]);
        writeLen = write(connfd, buffer, bufferLength);
        printf("writeLen %zd, 0x%x, 0x%x\r\n", writeLen, buffer[2], buffer[3]);
        if (writeLen < 0) {
            printf("发送失败\n");
            NSAlert* alert = [[NSAlert alloc] init];
            alert.messageText = [NSString stringWithFormat:@"发送失败"];
            [alert addButtonWithTitle:@"OK"];
            [alert runModal];
            close(connfd);

            return;
        }
        else
        {
            printf("发送成功\n");
            NSAlert* alert = [[NSAlert alloc] init];
            alert.messageText = [NSString stringWithFormat:@"发送成功"];
            [alert addButtonWithTitle:@"OK"];
            [alert runModal];
            close(connfd);
            return;
        }
    }
}
    
- (NSString*)freqToString: (int)freq {
    NSString* string = @"000";
    if (freq < 20) {
        
    }
    else if (freq < 4000) {
        string = [NSString stringWithFormat:@"%d", freq];
    } else {
        float f = (float)freq / 1000;
        if (freq%1000 != 0)
            string = [NSString stringWithFormat:@"%.1fk", f];
        else
            string = [NSString stringWithFormat:@"%dk",(int)f];
    }
    return string;
}


@end
