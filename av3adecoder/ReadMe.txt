信息技术 智能媒体编码 第3部分：沉浸式音频 第一阶段（广电/流媒体Profile） (以下简称AVS3-P3)参考编解码器使用说明

*  版本：AVS3A_RMv3.3.1
   1）该版本仅用于“百城千屏”端到端打通的集成和测试
   2）后续会发布更新版本代码

*  参考软件包含工具及说明：
    通用全码率音频编码工具: 支持全部码率下声道信号编码、对象信号编码、HOA信号编码
    元数据编码工具             : 支持元数据编码


1. 编码器
1.1 命令行选项
    avs3Encoder  [options]  [bitrate]  [samplingRate]  [inFileName]  [outFileName]
参数说明：
    inFileName：输入文件名（*.wav）
    outFileName：输出文件名（*.av3a）
    bitrate：编码速率（bps）
    samplingRate：输入信号采样率（kHz）
options说明：
    -nn_type: 神经网络模式配置，0代表main profile，1代表LC profile。
    -bitdepth：位深，16：16比特位深，24：24比特位深
    -mono：单声道模式
    -stereo：立体声模式
    -mc mcMode：多声道模式，mcMode为MC声道配置，例如MC_5_1_0（5.1）、MC_7_1_4（7.1.4）等
    -hoa M：HOA模式，M为HOA阶数
    -mix soundBedType：多声道与对象混合模式，soundBedType为0代表纯对象，1代表声床+对象混合信号，具体声床配置和速率配置见下方示例
    -meta_file filename：元数据文件名，文件名为filename
示例1：
    avs3Encoder -nn_type 0 -bitdepth 16 -mono 44000 48 test.wav test.av3a
    -- 单声道编码，main profile，输入信号位深16bit，采样率48kHz，编码速率44kbps，输入文件test.wav，输出码流文件test.av3a
示例2：
    avs3Encoder -nn_type 1 -bitdepth 16 -stereo 48000 48 test.wav test.av3a
    -- 立体声编码，LC profile，输入信号位深16bit，采样率48kHz，编码速率48kbps，输入文件test.wav，输出码流test.av3a
示例3：
    avs3Encoder -nn_type 1 -bitdepth 16 -mc MC_5_1_0 192000 48 test.wav test.av3a
    -- 多声道编码，格式5.1，LC profile，输入信号位深16bit，采样率48kHz，编码速率192kbps，输入文件test.wav，输出码流test.av3a
示例4：
    avs3Encoder -nn_type 1 -bitdepth 16 -mix 0 10 96000 0 48 test.wav test.av3a
    -- 混合信号编码，纯对象格式，对象数量10，LC profile，输入信号位深16bit，采样率48kHz，每个对象编码速率96kbps，总速率960kbps，输入文件test.wav，输出码流test.av3a
示例5：(5.1.4 +4 )(总的不大于16)
    avs3Encoder -nn_type 1 -bitdepth 16 -mix 1 MC_5_1_4 384000 4 44000 0 48 test.wav test.av3a
    -- 混合信号编码，声床+对象格式，声床格式5.1.4，对象数量4，LC profile，输入信号位深16bit，采样率48kHz，声床速率384kbps，每个对象速率44kbps，输入文件test.wav，输出码流test.av3a
示例6：
    avs3Encoder -nn_type 1 -bitdepth 16 -hoa 3 256000 48 test.wav test.av3a
    -- HOA编码，阶数为3，LC profile，输入信号位深16bit，采样率48kHz，编码速率256kbps，输入文件test.wav，输出码流文件test.av3a

2. 解码器
1.1 命令行选项
    avs3Decoder  [inFileName]  [outFileName]
参数说明：
    inFileName：输入文件名（*.av3a）
    outFileName：输出文件名（*.wav）
示例：
    avs3Decoder test.av3a test_dec.wav
    -- 对输入码流test.av3a进行解码，得到解码音频文件test_dec.wav