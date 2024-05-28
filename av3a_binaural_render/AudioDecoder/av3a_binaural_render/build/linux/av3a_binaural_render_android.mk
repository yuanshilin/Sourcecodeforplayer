##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
NDK=/root/android-ndk-r20b
#NDK=/home/wwzhang/android-ndk
TOOLCHAIN=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64

ARCH=arm64
CPU=armv8-a
API=24
CC=$(TOOLCHAIN)/bin/aarch64-linux-android$(API)-clang
CXX=$(TOOLCHAIN)/bin/aarch64-linux-android$(API)-clang++
SYSROOT=$(TOOLCHAIN)/sysroot
CROSS=aarch64-linux-android
CROSS_PREFIX=$(TOOLCHAIN)/bin/$(CROSS)
PREFIX=/home/arcvideo/ljin/ffmpeg-6.1/android/$(CPU)
OPTIMIZE_CFLAGS="-march=$(CPU)"


ProjectName            :=av3a_binaural_render
ConfigurationName      :=Release
# WorkspacePath          := "/mnt/data/remote/yfxu_coding/myworkspace"
# ProjectPath            := "/mnt/data/remote/yfxu_coding/AudioDecoder/av3a_binaural_render/build/linux"
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=root
Date                   :=12/26/23
CodeLitePath           :="/root/.codelite"
LinkerName             :=g++
SharedObjectLinkerName :=$(CXX) -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=../../../../bin/linux/lib$(ProjectName).so
# OutputFile             :=/mnt/data/local-disk1/yfxu/linux/lib$(ProjectName).so
Preprocessors          :=$(PreprocessorSwitch)HAVE_CONFIG_H 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="av3a_binaural_render.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            := -lstdc++ -Wl,--version-script=version-script.txt -Wl,--retain-symbols-file=retain_symbols.txt 
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../../ $(IncludeSwitch)../../../../VMFFramework/bin/VMFSDK/include/PlatForm $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
#CXX      := g++
#CC       := gcc
# CXXFLAGS := -std=c++11 -fPIC -g -Wall $(Preprocessors)
# CFLAGS   := -fPIC -g -Wall $(Preprocessors)
CXXFLAGS := -O3 -std=c++11 -fPIC -Wall $(Preprocessors)
CFLAGS   := -O3 -fPIC -Wall $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/av3a_binaural_render_binaural_render$(ObjectSuffix) $(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(ObjectSuffix) $(IntermediateDirectory)/av3a_binaural_render_adm$(ObjectSuffix) $(IntermediateDirectory)/av3a_binaural_render_metadata$(ObjectSuffix) $(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(ObjectSuffix) $(IntermediateDirectory)/av3a_binaural_render_xml_parser$(ObjectSuffix) $(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(ObjectSuffix) $(IntermediateDirectory)/av3a_binaural_render_interface$(ObjectSuffix) $(IntermediateDirectory)/pffft_pffft$(ObjectSuffix) $(IntermediateDirectory)/ambisonics_SHEval$(ObjectSuffix) \
	$(IntermediateDirectory)/core_ambisonic_binaural_decoder$(ObjectSuffix) $(IntermediateDirectory)/core_ambisonic_encoder$(ObjectSuffix) $(IntermediateDirectory)/core_ambisonic_rotator$(ObjectSuffix) $(IntermediateDirectory)/core_fft_manager$(ObjectSuffix) $(IntermediateDirectory)/core_listener$(ObjectSuffix) $(IntermediateDirectory)/core_ramp_processor$(ObjectSuffix) $(IntermediateDirectory)/core_sound_source$(ObjectSuffix) $(IntermediateDirectory)/core_spherical_harmonic_hrir$(ObjectSuffix) $(IntermediateDirectory)/core_static_convolver$(ObjectSuffix) $(IntermediateDirectory)/simd_simd_utils$(ObjectSuffix) \
	$(IntermediateDirectory)/hrtf_database_hrtf_assets$(ObjectSuffix) $(IntermediateDirectory)/libsamplerate_samplerate$(ObjectSuffix) $(IntermediateDirectory)/libsamplerate_src_linear$(ObjectSuffix) $(IntermediateDirectory)/libsamplerate_src_sinc$(ObjectSuffix) $(IntermediateDirectory)/libsamplerate_src_zoh$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
# @$(MakeDirCommand) "/mnt/data/remote/yfxu_coding/myworkspace/.build-release"
# @echo rebuilt > "/mnt/data/remote/yfxu_coding/myworkspace/.build-release/av3a_binaural_render"

$(IntermediateDirectory)/.d:
	@test -d ./Release || $(MakeDirCommand) ./Release

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/av3a_binaural_render_binaural_render$(ObjectSuffix): ../../binaural_render.cpp $(IntermediateDirectory)/av3a_binaural_render_binaural_render$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../binaural_render.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_binaural_render$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_binaural_render$(DependSuffix): ../../binaural_render.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_binaural_render$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_binaural_render$(DependSuffix) -MM "../../binaural_render.cpp"

$(IntermediateDirectory)/av3a_binaural_render_binaural_render$(PreprocessSuffix): ../../binaural_render.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_binaural_render$(PreprocessSuffix) "../../binaural_render.cpp"

$(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(ObjectSuffix): ../../stream_renderer.cpp $(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../stream_renderer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(DependSuffix): ../../stream_renderer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(DependSuffix) -MM "../../stream_renderer.cpp"

$(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(PreprocessSuffix): ../../stream_renderer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(PreprocessSuffix) "../../stream_renderer.cpp"

$(IntermediateDirectory)/av3a_binaural_render_adm$(ObjectSuffix): ../../adm.cpp $(IntermediateDirectory)/av3a_binaural_render_adm$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../adm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_adm$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_adm$(DependSuffix): ../../adm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_adm$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_adm$(DependSuffix) -MM "../../adm.cpp"

$(IntermediateDirectory)/av3a_binaural_render_adm$(PreprocessSuffix): ../../adm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_adm$(PreprocessSuffix) "../../adm.cpp"

$(IntermediateDirectory)/av3a_binaural_render_metadata$(ObjectSuffix): ../../metadata.cpp $(IntermediateDirectory)/av3a_binaural_render_metadata$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../metadata.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_metadata$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_metadata$(DependSuffix): ../../metadata.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_metadata$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_metadata$(DependSuffix) -MM "../../metadata.cpp"

$(IntermediateDirectory)/av3a_binaural_render_metadata$(PreprocessSuffix): ../../metadata.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_metadata$(PreprocessSuffix) "../../metadata.cpp"

$(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(ObjectSuffix): ../../metadata_parser.cpp $(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../metadata_parser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(DependSuffix): ../../metadata_parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(DependSuffix) -MM "../../metadata_parser.cpp"

$(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(PreprocessSuffix): ../../metadata_parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(PreprocessSuffix) "../../metadata_parser.cpp"

$(IntermediateDirectory)/av3a_binaural_render_xml_parser$(ObjectSuffix): ../../xml_parser.cpp $(IntermediateDirectory)/av3a_binaural_render_xml_parser$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../xml_parser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_xml_parser$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_xml_parser$(DependSuffix): ../../xml_parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_xml_parser$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_xml_parser$(DependSuffix) -MM "../../xml_parser.cpp"

$(IntermediateDirectory)/av3a_binaural_render_xml_parser$(PreprocessSuffix): ../../xml_parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_xml_parser$(PreprocessSuffix) "../../xml_parser.cpp"

$(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(ObjectSuffix): ../../avs3_audio.cpp $(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../avs3_audio.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(DependSuffix): ../../avs3_audio.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(DependSuffix) -MM "../../avs3_audio.cpp"

$(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(PreprocessSuffix): ../../avs3_audio.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(PreprocessSuffix) "../../avs3_audio.cpp"

$(IntermediateDirectory)/av3a_binaural_render_interface$(ObjectSuffix): ../../interface.cpp $(IntermediateDirectory)/av3a_binaural_render_interface$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../interface.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/av3a_binaural_render_interface$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/av3a_binaural_render_interface$(DependSuffix): ../../interface.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/av3a_binaural_render_interface$(ObjectSuffix) -MF$(IntermediateDirectory)/av3a_binaural_render_interface$(DependSuffix) -MM "../../interface.cpp"

$(IntermediateDirectory)/av3a_binaural_render_interface$(PreprocessSuffix): ../../interface.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/av3a_binaural_render_interface$(PreprocessSuffix) "../../interface.cpp"

$(IntermediateDirectory)/pffft_pffft$(ObjectSuffix): ../../ext/pffft/pffft.c $(IntermediateDirectory)/pffft_pffft$(DependSuffix)
	$(CC) $(SourceSwitch) "../../ext/pffft/pffft.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/pffft_pffft$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/pffft_pffft$(DependSuffix): ../../ext/pffft/pffft.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/pffft_pffft$(ObjectSuffix) -MF$(IntermediateDirectory)/pffft_pffft$(DependSuffix) -MM "../../ext/pffft/pffft.c"

$(IntermediateDirectory)/pffft_pffft$(PreprocessSuffix): ../../ext/pffft/pffft.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/pffft_pffft$(PreprocessSuffix) "../../ext/pffft/pffft.c"

$(IntermediateDirectory)/ambisonics_SHEval$(ObjectSuffix): ../../ambisonics/SHEval.cpp $(IntermediateDirectory)/ambisonics_SHEval$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../ambisonics/SHEval.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ambisonics_SHEval$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ambisonics_SHEval$(DependSuffix): ../../ambisonics/SHEval.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ambisonics_SHEval$(ObjectSuffix) -MF$(IntermediateDirectory)/ambisonics_SHEval$(DependSuffix) -MM "../../ambisonics/SHEval.cpp"

$(IntermediateDirectory)/ambisonics_SHEval$(PreprocessSuffix): ../../ambisonics/SHEval.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ambisonics_SHEval$(PreprocessSuffix) "../../ambisonics/SHEval.cpp"

$(IntermediateDirectory)/core_ambisonic_binaural_decoder$(ObjectSuffix): ../../core/ambisonic_binaural_decoder.cpp $(IntermediateDirectory)/core_ambisonic_binaural_decoder$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/ambisonic_binaural_decoder.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_ambisonic_binaural_decoder$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_ambisonic_binaural_decoder$(DependSuffix): ../../core/ambisonic_binaural_decoder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_ambisonic_binaural_decoder$(ObjectSuffix) -MF$(IntermediateDirectory)/core_ambisonic_binaural_decoder$(DependSuffix) -MM "../../core/ambisonic_binaural_decoder.cpp"

$(IntermediateDirectory)/core_ambisonic_binaural_decoder$(PreprocessSuffix): ../../core/ambisonic_binaural_decoder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_ambisonic_binaural_decoder$(PreprocessSuffix) "../../core/ambisonic_binaural_decoder.cpp"

$(IntermediateDirectory)/core_ambisonic_encoder$(ObjectSuffix): ../../core/ambisonic_encoder.cpp $(IntermediateDirectory)/core_ambisonic_encoder$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/ambisonic_encoder.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_ambisonic_encoder$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_ambisonic_encoder$(DependSuffix): ../../core/ambisonic_encoder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_ambisonic_encoder$(ObjectSuffix) -MF$(IntermediateDirectory)/core_ambisonic_encoder$(DependSuffix) -MM "../../core/ambisonic_encoder.cpp"

$(IntermediateDirectory)/core_ambisonic_encoder$(PreprocessSuffix): ../../core/ambisonic_encoder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_ambisonic_encoder$(PreprocessSuffix) "../../core/ambisonic_encoder.cpp"

$(IntermediateDirectory)/core_ambisonic_rotator$(ObjectSuffix): ../../core/ambisonic_rotator.cpp $(IntermediateDirectory)/core_ambisonic_rotator$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/ambisonic_rotator.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_ambisonic_rotator$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_ambisonic_rotator$(DependSuffix): ../../core/ambisonic_rotator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_ambisonic_rotator$(ObjectSuffix) -MF$(IntermediateDirectory)/core_ambisonic_rotator$(DependSuffix) -MM "../../core/ambisonic_rotator.cpp"

$(IntermediateDirectory)/core_ambisonic_rotator$(PreprocessSuffix): ../../core/ambisonic_rotator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_ambisonic_rotator$(PreprocessSuffix) "../../core/ambisonic_rotator.cpp"

$(IntermediateDirectory)/core_fft_manager$(ObjectSuffix): ../../core/fft_manager.cpp $(IntermediateDirectory)/core_fft_manager$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/fft_manager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_fft_manager$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_fft_manager$(DependSuffix): ../../core/fft_manager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_fft_manager$(ObjectSuffix) -MF$(IntermediateDirectory)/core_fft_manager$(DependSuffix) -MM "../../core/fft_manager.cpp"

$(IntermediateDirectory)/core_fft_manager$(PreprocessSuffix): ../../core/fft_manager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_fft_manager$(PreprocessSuffix) "../../core/fft_manager.cpp"

$(IntermediateDirectory)/core_listener$(ObjectSuffix): ../../core/listener.cpp $(IntermediateDirectory)/core_listener$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/listener.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_listener$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_listener$(DependSuffix): ../../core/listener.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_listener$(ObjectSuffix) -MF$(IntermediateDirectory)/core_listener$(DependSuffix) -MM "../../core/listener.cpp"

$(IntermediateDirectory)/core_listener$(PreprocessSuffix): ../../core/listener.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_listener$(PreprocessSuffix) "../../core/listener.cpp"

$(IntermediateDirectory)/core_ramp_processor$(ObjectSuffix): ../../core/ramp_processor.cpp $(IntermediateDirectory)/core_ramp_processor$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/ramp_processor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_ramp_processor$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_ramp_processor$(DependSuffix): ../../core/ramp_processor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_ramp_processor$(ObjectSuffix) -MF$(IntermediateDirectory)/core_ramp_processor$(DependSuffix) -MM "../../core/ramp_processor.cpp"

$(IntermediateDirectory)/core_ramp_processor$(PreprocessSuffix): ../../core/ramp_processor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_ramp_processor$(PreprocessSuffix) "../../core/ramp_processor.cpp"

$(IntermediateDirectory)/core_sound_source$(ObjectSuffix): ../../core/sound_source.cpp $(IntermediateDirectory)/core_sound_source$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/sound_source.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_sound_source$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_sound_source$(DependSuffix): ../../core/sound_source.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_sound_source$(ObjectSuffix) -MF$(IntermediateDirectory)/core_sound_source$(DependSuffix) -MM "../../core/sound_source.cpp"

$(IntermediateDirectory)/core_sound_source$(PreprocessSuffix): ../../core/sound_source.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_sound_source$(PreprocessSuffix) "../../core/sound_source.cpp"

$(IntermediateDirectory)/core_spherical_harmonic_hrir$(ObjectSuffix): ../../core/spherical_harmonic_hrir.cpp $(IntermediateDirectory)/core_spherical_harmonic_hrir$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/spherical_harmonic_hrir.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_spherical_harmonic_hrir$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_spherical_harmonic_hrir$(DependSuffix): ../../core/spherical_harmonic_hrir.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_spherical_harmonic_hrir$(ObjectSuffix) -MF$(IntermediateDirectory)/core_spherical_harmonic_hrir$(DependSuffix) -MM "../../core/spherical_harmonic_hrir.cpp"

$(IntermediateDirectory)/core_spherical_harmonic_hrir$(PreprocessSuffix): ../../core/spherical_harmonic_hrir.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_spherical_harmonic_hrir$(PreprocessSuffix) "../../core/spherical_harmonic_hrir.cpp"

$(IntermediateDirectory)/core_static_convolver$(ObjectSuffix): ../../core/static_convolver.cpp $(IntermediateDirectory)/core_static_convolver$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../core/static_convolver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/core_static_convolver$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/core_static_convolver$(DependSuffix): ../../core/static_convolver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/core_static_convolver$(ObjectSuffix) -MF$(IntermediateDirectory)/core_static_convolver$(DependSuffix) -MM "../../core/static_convolver.cpp"

$(IntermediateDirectory)/core_static_convolver$(PreprocessSuffix): ../../core/static_convolver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/core_static_convolver$(PreprocessSuffix) "../../core/static_convolver.cpp"

$(IntermediateDirectory)/simd_simd_utils$(ObjectSuffix): ../../ext/simd/simd_utils.cc $(IntermediateDirectory)/simd_simd_utils$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../ext/simd/simd_utils.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/simd_simd_utils$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/simd_simd_utils$(DependSuffix): ../../ext/simd/simd_utils.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/simd_simd_utils$(ObjectSuffix) -MF$(IntermediateDirectory)/simd_simd_utils$(DependSuffix) -MM "../../ext/simd/simd_utils.cc"

$(IntermediateDirectory)/simd_simd_utils$(PreprocessSuffix): ../../ext/simd/simd_utils.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/simd_simd_utils$(PreprocessSuffix) "../../ext/simd/simd_utils.cc"

$(IntermediateDirectory)/hrtf_database_hrtf_assets$(ObjectSuffix): ../../ext/hrtf_database/hrtf_assets.cpp $(IntermediateDirectory)/hrtf_database_hrtf_assets$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "../../ext/hrtf_database/hrtf_assets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/hrtf_database_hrtf_assets$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/hrtf_database_hrtf_assets$(DependSuffix): ../../ext/hrtf_database/hrtf_assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/hrtf_database_hrtf_assets$(ObjectSuffix) -MF$(IntermediateDirectory)/hrtf_database_hrtf_assets$(DependSuffix) -MM "../../ext/hrtf_database/hrtf_assets.cpp"

$(IntermediateDirectory)/hrtf_database_hrtf_assets$(PreprocessSuffix): ../../ext/hrtf_database/hrtf_assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/hrtf_database_hrtf_assets$(PreprocessSuffix) "../../ext/hrtf_database/hrtf_assets.cpp"

$(IntermediateDirectory)/libsamplerate_samplerate$(ObjectSuffix): ../../ext/libsamplerate/samplerate.c $(IntermediateDirectory)/libsamplerate_samplerate$(DependSuffix)
	$(CC) $(SourceSwitch) "../../ext/libsamplerate/samplerate.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/libsamplerate_samplerate$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/libsamplerate_samplerate$(DependSuffix): ../../ext/libsamplerate/samplerate.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/libsamplerate_samplerate$(ObjectSuffix) -MF$(IntermediateDirectory)/libsamplerate_samplerate$(DependSuffix) -MM "../../ext/libsamplerate/samplerate.c"

$(IntermediateDirectory)/libsamplerate_samplerate$(PreprocessSuffix): ../../ext/libsamplerate/samplerate.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/libsamplerate_samplerate$(PreprocessSuffix) "../../ext/libsamplerate/samplerate.c"

$(IntermediateDirectory)/libsamplerate_src_linear$(ObjectSuffix): ../../ext/libsamplerate/src_linear.c $(IntermediateDirectory)/libsamplerate_src_linear$(DependSuffix)
	$(CC) $(SourceSwitch) "../../ext/libsamplerate/src_linear.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/libsamplerate_src_linear$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/libsamplerate_src_linear$(DependSuffix): ../../ext/libsamplerate/src_linear.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/libsamplerate_src_linear$(ObjectSuffix) -MF$(IntermediateDirectory)/libsamplerate_src_linear$(DependSuffix) -MM "../../ext/libsamplerate/src_linear.c"

$(IntermediateDirectory)/libsamplerate_src_linear$(PreprocessSuffix): ../../ext/libsamplerate/src_linear.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/libsamplerate_src_linear$(PreprocessSuffix) "../../ext/libsamplerate/src_linear.c"

$(IntermediateDirectory)/libsamplerate_src_sinc$(ObjectSuffix): ../../ext/libsamplerate/src_sinc.c $(IntermediateDirectory)/libsamplerate_src_sinc$(DependSuffix)
	$(CC) $(SourceSwitch) "../../ext/libsamplerate/src_sinc.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/libsamplerate_src_sinc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/libsamplerate_src_sinc$(DependSuffix): ../../ext/libsamplerate/src_sinc.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/libsamplerate_src_sinc$(ObjectSuffix) -MF$(IntermediateDirectory)/libsamplerate_src_sinc$(DependSuffix) -MM "../../ext/libsamplerate/src_sinc.c"

$(IntermediateDirectory)/libsamplerate_src_sinc$(PreprocessSuffix): ../../ext/libsamplerate/src_sinc.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/libsamplerate_src_sinc$(PreprocessSuffix) "../../ext/libsamplerate/src_sinc.c"

$(IntermediateDirectory)/libsamplerate_src_zoh$(ObjectSuffix): ../../ext/libsamplerate/src_zoh.c $(IntermediateDirectory)/libsamplerate_src_zoh$(DependSuffix)
	$(CC) $(SourceSwitch) "../../ext/libsamplerate/src_zoh.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/libsamplerate_src_zoh$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/libsamplerate_src_zoh$(DependSuffix): ../../ext/libsamplerate/src_zoh.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/libsamplerate_src_zoh$(ObjectSuffix) -MF$(IntermediateDirectory)/libsamplerate_src_zoh$(DependSuffix) -MM "../../ext/libsamplerate/src_zoh.c"

$(IntermediateDirectory)/libsamplerate_src_zoh$(PreprocessSuffix): ../../ext/libsamplerate/src_zoh.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/libsamplerate_src_zoh$(PreprocessSuffix) "../../ext/libsamplerate/src_zoh.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_binaural_render$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_binaural_render$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_binaural_render$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_stream_renderer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_adm$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_adm$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_adm$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_metadata$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_metadata$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_metadata$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_metadata_parser$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_xml_parser$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_xml_parser$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_xml_parser$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_avs3_audio$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_interface$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_interface$(DependSuffix)
	$(RM) $(IntermediateDirectory)/av3a_binaural_render_interface$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/pffft_pffft$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/pffft_pffft$(DependSuffix)
	$(RM) $(IntermediateDirectory)/pffft_pffft$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/ambisonics_SHEval$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/ambisonics_SHEval$(DependSuffix)
	$(RM) $(IntermediateDirectory)/ambisonics_SHEval$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_binaural_decoder$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_binaural_decoder$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_binaural_decoder$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_encoder$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_encoder$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_encoder$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_rotator$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_rotator$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_ambisonic_rotator$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_fft_manager$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_fft_manager$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_fft_manager$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_listener$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_listener$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_listener$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_ramp_processor$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_ramp_processor$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_ramp_processor$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_sound_source$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_sound_source$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_sound_source$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_spherical_harmonic_hrir$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_spherical_harmonic_hrir$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_spherical_harmonic_hrir$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/core_static_convolver$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/core_static_convolver$(DependSuffix)
	$(RM) $(IntermediateDirectory)/core_static_convolver$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/simd_simd_utils$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/simd_simd_utils$(DependSuffix)
	$(RM) $(IntermediateDirectory)/simd_simd_utils$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/hrtf_database_hrtf_assets$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/hrtf_database_hrtf_assets$(DependSuffix)
	$(RM) $(IntermediateDirectory)/hrtf_database_hrtf_assets$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_samplerate$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_samplerate$(DependSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_samplerate$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_linear$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_linear$(DependSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_linear$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_sinc$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_sinc$(DependSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_sinc$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_zoh$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_zoh$(DependSuffix)
	$(RM) $(IntermediateDirectory)/libsamplerate_src_zoh$(PreprocessSuffix)
	$(RM) $(OutputFile)
# (RM) "../../../../myworkspace/.build-release/av3a_binaural_render"


