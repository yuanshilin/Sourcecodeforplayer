//
//  ViewController.m
//  AudioPlayer
//
//  Created by develop on 2024/2/5.
//

#import "ViewController.h"
#import "FilterEngine.h"
#include "SDL2/SDL.h"

#define MAX_GAIN 15
#define MIN_GAIN -15
#define DEFAULT_Q 1.0f / sqrt(2.0f)

#define LABEL_WIDTH 40.0f
#define LABEL_HEIGHT 20.0f
#define MARGIN 10.0f

static int freqs[] = {20,25,31,40,50,63,80,100,118, 156,196,264,326, 408,524,641,800,998,1322,1579, 2000,2477,3150, 4000, 5000, 6300,8000, 10000, 12500, 16000, 20000};

typedef struct AudioBuffer {
    int len;
    int pullLen;
    uint8_t *data;
} AudioBuffer;

@interface ViewController () <NSTextFieldDelegate>
{
    SDL_AudioSpec audioSpec;
    ;
}

@property(nonatomic) dispatch_queue_t queue;
@property(nonatomic) Boolean    interruptionRequested;

@property(strong) NSString* filePath;
@property(assign) uint16_t  enabled_channel_bit;

@property (strong) IBOutlet NSTextField *filePathTextField;
@property (strong) IBOutlet NSTextField *channelBitTextField;

@property (strong) IBOutlet NSButton *playButton;

@property (strong) IBOutlet NSView *hzSliderView;

@property (strong) IBOutlet NSButton *resetButton;

@property (strong) NSMutableDictionary *filters;
@property (strong) NSMutableDictionary *filterParams;
@property (strong) NSLock              *filterLock;

@end


AudioBuffer audioBuffer;
Sint8* temp = NULL;
uint64_t total = 0;
void* filterEngine = NULL;
FILE * fbefore = NULL;
FILE *fout = NULL;

uint64_t GetCurrentTimestamp(void) {
#if 1
    uint64_t now_time;
    now_time = CVGetCurrentHostTime() * 1000 / CVGetHostClockFrequency();
    return now_time;
#else
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now_mstime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    return now_mstime;
#endif
}

// 等待音频设备回调(会回调多次)
void pull_audio_data(void *userdata,
                     // 需要往stream中填充PCM数据
                     Uint8 *stream,
                     // 希望填充的大小(samples * format * channels / 8)
                     int len
                     ) {
    ViewController* ctrl = (__bridge ViewController *)(userdata);
    AudioBuffer *buffer = &audioBuffer;
    SDL_memset(stream, 0, len);
    if (buffer->len <= 0) {
        return;
    }
    buffer->pullLen = len > buffer->len ? buffer->len : len;
//    NSLog(@"before-buffer->len: %d, buffer->pullLen %d, len: %d", buffer->len, buffer->pullLen, len);
    
    Sint8* dst = (Sint8*)buffer->data;

    if (fbefore) {
        fwrite(dst, 1, buffer->pullLen, fbefore);
    }
    dst = FilterAudio(filterEngine, (Sint8*)buffer->data, buffer->pullLen);

    if (fout) {
        fwrite(dst, 1, buffer->pullLen, fout);
    }
    SDL_MixAudio(stream, (UInt8 *)dst, buffer->pullLen, SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
//    NSLog(@"buffer->len: %d, buffer->pullLen %d", buffer->len, buffer->pullLen);
}



@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    SDL_memset(&audioSpec, 0, sizeof(audioSpec));
    self.filters = [NSMutableDictionary dictionaryWithCapacity:10];
    self.filterParams = [NSMutableDictionary dictionaryWithCapacity:10];
    
    self.filePathTextField.delegate = self;
    self.filePathTextField.stringValue = @"/Users/develop/Downloads/AudioFile/sample-15s.wav";
    self.filePath = self.filePathTextField.stringValue;
    
    self.enabled_channel_bit = 0xFFFF;
    self.channelBitTextField.contentType = NSTextContentTypeTelephoneNumber;
    self.channelBitTextField.delegate = self;
    
    [self setupSliders];

    self.filterLock = [[NSLock alloc] init];
    self.queue = dispatch_queue_create("AUDIOPLAY", NULL);
    
    
    
    // Do any additional setup after loading the view.
}

- (void)setupSliders {
    //    NSLog(@"view size %@", NSStringFromSize(self.hzSliderView.frame.size));
        self.hzSliderView.wantsLayer = YES;
        self.hzSliderView.layer.backgroundColor = [NSColor lightGrayColor].CGColor;
        
        float slider_height = self.hzSliderView.frame.size.height - MARGIN*3 - LABEL_HEIGHT;
    //    float slider_center_y = MARGIN + slider_height / 2;
        int slider_count = sizeof(freqs) / sizeof(int);
        NSLog(@"count %d", slider_count);
        float slider_offset = self.hzSliderView.frame.size.width / (slider_count + 1);

        for (int i = 0; i < slider_count; i++) {
            
            float slider_center_x = slider_offset * (i + 1);
            NSTextField* text = [NSTextField textFieldWithString: [self freqToString:freqs[i]]];
            text.editable = NO;
            text.frame = CGRectMake(slider_center_x - LABEL_WIDTH / 2, MARGIN, LABEL_WIDTH, LABEL_HEIGHT);
            text.alignment = NSTextAlignmentCenter;
            [self.hzSliderView addSubview:text];
            
            NSSlider* slider = [NSSlider sliderWithValue:0 minValue:MIN_GAIN maxValue:MAX_GAIN target:self action:@selector(hzSlider_Changed:)];
            slider.continuous = NO;
            slider.tag = freqs[i];
            slider.wantsLayer = YES;
    //        slider.layer.backgroundColor = [NSColor greenColor].CGColor;
            slider.vertical = true;
    //        NSLog(@"slider frame %@", NSStringFromRect(slider.frame));
            float slider_width = slider.frame.size.height;
            slider.frame = CGRectMake(slider_center_x - slider_width / 2, MARGIN*2 + LABEL_HEIGHT, slider_width, slider_height);
    //        NSLog(@"slider frame %@", NSStringFromRect(slider.frame));
            [self.hzSliderView addSubview:slider];
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

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
//    UInt16 t;
    // Update the view, if already loaded.
}

- (void)playWithWavFile:(NSString*)wavFile
{
    fbefore = fopen("/Users/develop/Downloads/test/test_out_before.pcm", "wb");
    fout = fopen("/Users/develop/Downloads/test/test_out_after.pcm", "wb");

    dispatch_async(self.queue, ^{
        CreateFilterEngine(&filterEngine);
        if (SDL_Init(SDL_INIT_AUDIO)) {
            return;
        }
        self.interruptionRequested = false;
        SDL_AudioSpec spec;
        Uint8 *data = NULL;
        UInt32 len = 0;
        if (!SDL_LoadWAV([wavFile UTF8String], &spec, &data, &len)) {
            NSLog(@"SDL_LoadWAV Error: %s", SDL_GetError());
            SDL_Quit();
            return;
        }
        spec.samples = 1024;
        spec.callback = pull_audio_data;
        
        int tempLen = spec.samples * spec.channels * 10;
        if (temp == NULL)
            temp = (Sint8*)malloc(tempLen);
        memset(temp, 0, tempLen);

        SDL_memcpy(&self->audioSpec, &spec, sizeof(audioSpec));
        
        AudioParam param = {0};
        param.format = spec.format;
        param.channels = spec.channels;
        param.freq = spec.freq;
        param.samples = spec.samples;
        
        NSString* configFile = @"/Users/develop/Downloads/test/filter.json";
        
        bool b = [[NSFileManager defaultManager] isReadableFileAtPath:configFile];
        
        StartFilterEngine(filterEngine, &param, [configFile UTF8String]);
        StartDebug(filterEngine);
        
        audioBuffer.data = data;
        audioBuffer.len = len;
        spec.userdata = (__bridge void *)(self);
        if (SDL_OpenAudio(&spec, NULL)) {
            NSLog(@"SDL_OpenAudio Error: %s", SDL_GetError());
            SDL_Quit();
            return;
        }
        
        int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
        NSLog(@"sampleSize %d", sampleSize);
        int bytesPerSample = (sampleSize * spec.channels) >> 3;
        SDL_PauseAudio(0);
        while (!self.interruptionRequested) {
//            NSLog(@"audioBuffer.len %d, pull len %d", audioBuffer.len, audioBuffer.pullLen);

            if (audioBuffer.len > 0) {
                int samples = audioBuffer.pullLen / bytesPerSample;
                int ms = samples * 1000 / spec.freq;
//                NSLog(@"delay %d", ms);
                SDL_Delay(ms);

                continue;
            }
            if (audioBuffer.len <= 0) {
                int samples = audioBuffer.pullLen / bytesPerSample;
                int ms = samples * 1000 / spec.freq;
//                NSLog(@"delay %d", ms);
                SDL_Delay(ms);
                break;
            }
        }
        NSLog(@"end");
        SDL_FreeWAV(data);
        SDL_CloseAudio();
        SDL_Quit();
        StopFilterEngine(filterEngine);
        DestroyFilterEngine(filterEngine);
        filterEngine = NULL;
    });

}

- (void)controlTextDidChange:(NSNotification *)obj
{
    NSTextField* textfield = obj.object;
    NSLog(@"[%s] %@", __FUNCTION__, textfield.stringValue);

    if ([textfield isEqualTo:self.filePathTextField]) {
        self.filePath = textfield.stringValue;
    } else if ([textfield isEqualTo:self.channelBitTextField]) {
        self.enabled_channel_bit = textfield.intValue;
    }
}

- (IBAction)playButtonClicked:(id)sender {
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:self.filePath]) {
        NSLog(@"%@ not exist", self.filePath);
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"%@ not exist", self.filePath];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    }
    if ([[self.filePath pathExtension] isEqualToString:@"wav"]) {
        [self playWithWavFile:self.filePath];
    } else {
        NSLog(@"file type [%@] not support", [self.filePath pathExtension]);
        NSAlert* alert = [[NSAlert alloc] init];
        alert.messageText = [NSString stringWithFormat:@"file type [%@] not support", [self.filePath pathExtension]];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
    }
}

- (IBAction)resetButtonClicked:(id)sender {

    ResetFilter(filterEngine);
    
    for (NSView* view in self.hzSliderView.subviews) {
        if ([view.class isEqualTo:[NSSlider class]]) {
            ((NSSlider*)view).intValue = 0;
        }
    }
}

- (void)hzSlider_Changed:(id)sender {
    NSSlider* slider = sender;
    NSLog(@"[%s] %d: %d", __FUNCTION__, (int)slider.tag, slider.intValue);
    
    EqulizerParam param = {0};
    Filter_Type type = Filter_Type_BYPASS;
    if (slider.intValue > 0) {
        type = Filter_Type_2ND_PEAK;
    } else if (slider.intValue < 0) {
        type = Filter_Type_2ND_NOTCH;
    }
    param.quality_factor = DEFAULT_Q;
    param.centre_freq = (uint32_t)slider.tag;
    param.type = type;
    param.dbgain = slider.intValue;
    param.enabled_channel_bit = self.enabled_channel_bit;
    
    AddFilter(filterEngine, &param);

}

@end
