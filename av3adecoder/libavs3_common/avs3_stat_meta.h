
#ifndef _AVS3_STAT_META_H_
#define _AVS3_STAT_META_H_

#include <stdint.h>

//VREXT
typedef struct EqEffectStructure {
    int16_t eqType;
    float   eqFc;
    float   eqQ;
    float   eqGain;
}EqEffect, *EqEffectHandle;

typedef struct AudioEffectStructure {
    int16_t hasEQExist;
    int16_t hasDRCExist;
    int16_t hasGainExist;

    int16_t effectChain;
    int16_t numEqband;

    EqEffect eqEffect[16];

    float attackTime;
    float releaseTime;
    float threshold;
    float preGain;
    float postGain;
    float ratio;
    float effectGain;
}AudioEffect, *AudioEffectHandle;

typedef struct VertexStructure {
    float x;
    float y;
    float z;
}Vertex, *VertexHandle;

typedef struct SurfaceStructure {
    int16_t material;

    float absorption[8];
    float scattering[8];

    int16_t numVertices;

    Vertex vertex[32];
}Surface, *SurfaceHandle;

typedef struct RenderInfoStructure {
    int16_t targetDevice;
    int16_t hrtfType;

    int16_t headphoneType[16];

    AudioEffect audioEffect;
}RenderInfo, *RenderInfoHandle;

typedef struct AcousticEnvStructure {
    int16_t hasEarlyReflectionGain;
    int16_t hasLateReverbGain;
    int16_t reverbType;

    float earlyReflectionGain;
    float lateReverbGain;

    int16_t lowFreqProFlag;

    int16_t convolutionReverbType;

    int16_t numSurface;

    Surface surface[8];
}AcousticEnv, *AcousticEnvHandle;

typedef struct Avs3VrExtMetaDataStructure {
    int16_t hasAcousticEnv;
    int16_t hasRenderInfo;

    int16_t ambisonicOrder;

    AcousticEnv acousticEnv;
    RenderInfo  renderInfo;
}Avs3VrExtL1MetaData, *Avs3VrExtL1MetaDataHandle;

// ADM
typedef struct DirectSpeakersPositionStructure {

    float azimuth;
    float elevation;
    float distance;

    int16_t screenEdgeLock;
}DirectSpeakersPosition, *DirectSpeakersPositionHandle;

typedef struct AudioObjectInteractionStructure {
    int16_t onOffInteract;
    int16_t gainInteract;
    int16_t positionInteract;

    // gainInteract
    int16_t gainUnit;
    float gainInteractionRange_Min;
    float gainInteractionRange_Max;

    // positionInteract
    int16_t cartesian;

    // positionInteract cartesian
    float interactionRange_Xmin;
    float interactionRange_Xmax;
    float interactionRange_Ymin;
    float interactionRange_Ymax;
    float interactionRange_Zmin;
    float interactionRange_Zmax;

    // positionInteract polar
    float interactionRange_azimuthMin;
    float interactionRange_azimuthMax;
    float interactionRange_elevationMin;
    float interactionRange_elevationMax;
    float interactionRange_distanceMin;
    float interactionRange_distanceMax;
}AudioObjectInteraction, *AudioObjectInteractionHandle;

typedef struct DialogueStructure {
    int16_t dialogueAttribute;
    int16_t dialogueType;
}Dialogue, *DialogueHandle;

typedef struct AudioProgrammeRefScreenStructure {
    int16_t hasCartesian;
    int16_t aspectRatio;
    
    // polar
    float Position_azimuth;
    float Position_elevation;
    float Position_distance;
    float polarScreenWidth;

    // cartesian
    float Position_X;
    float Position_Y;    
    float Position_Z;
    float CartesianScreenWidth;
}AudioProgrammeRefScreen, *AudioProgrammeRefScreenHandle;

typedef struct LoudnessMetadataStructure {
    int16_t hasIntegratedLoudness;
    int16_t hasLoudnessRange;
    int16_t hasMaxTruePeak;
    int16_t hasMaxMomentary;
    int16_t hasMaxShortTerm;
    int16_t hasDialogueLoudness;

    float integratedLoudness;
    float loudnessRange;
    float maxTruePeak;
    float maxMomentary;
    float maxShortTerm;
    float dialogueLoudness;
}LoudnessMetadata, *LoudnessMetadataHandle;

typedef struct AudioChannelFormatStructure {
    int16_t channelFormatIdx;

    int16_t hasChannelGain;

    int16_t gainUnit;
    float channelGain;

    // typeLabel  == 2
    float MatrixCoef[32];

    // typeLabel  == 1(DirectSpeakers)
    // packFormatID == 0x3f
    DirectSpeakersPosition directSpeakersPositionData;
}AudioChannelFormat, *AudioChannelFormatHandle;

typedef struct AudioPackFormatStructure {
    int16_t packFormatIdx;

    int16_t hasImportance;
    int16_t hasChannelReuse;

    int16_t importance;

    int16_t typeLabel;
    int16_t packFormatID;
    float   absoluteDistance;
    
    // typeLabel==4
    int16_t normalization;
    float   nfcRefDist;
    int16_t screenRef;
    int16_t hoaOrder;

    // typeLabel==2
    int16_t numMatrixOutputChannel;

    // for numMatrixOutputChannel
    DirectSpeakersPosition directSpeakersPositionData[32];

    // hasChannelReuse==0
    int16_t packFormatStartIdx;

    int16_t numChannels;
    int16_t  refChannelIdx[32];

    int16_t transChRef[32];
}AudioPackFormat, *AudioPackFormatHandle;

typedef struct AudioObjectStructure {
    int16_t objectIdx;

    int16_t hasAudioObjectLanguage;
    int16_t hasDialogue;
    int16_t hasImportance;
    int16_t hasDisableDucking;
    int16_t hasInteract;
    int16_t hasGain;
    int16_t hasHeadLocked;
    int16_t hasMute;

    int16_t ObjectName[24]; //24*6bit

    int16_t audioObjectLanguage;

    Dialogue dialogueData;

    int16_t importance;

    // hasInteract
    AudioObjectInteraction audioObjectInteractionData;

    // hasGain
    int16_t gainUnit;
    float gain;

    int16_t numPacks;

    int16_t refPackFormatIdx[8];
}AudioObject, *AudioObjectHandle;

typedef struct AudioContentStructure {
    int16_t contentIdx;

    int16_t hasAudioContentLanguage;
    int16_t hasLoudnessMetadata;
    int16_t hasDialogue;
    int16_t hasNumComplementaryObjectGroup;

    int16_t  audioContentLanguage;
    LoudnessMetadata loudnessData;

    Dialogue  dialogueData;

    int16_t numComplementaryObjectGroup;
    int16_t numComplementaryObject[4];

    int16_t ComplementaryObjectIdx[4][8];

    int16_t numObjects;

    int16_t refObjectIdx[8];
}AudioContent, *AudioContentHandle;

typedef struct AudioProgrammeStructure {
    int16_t hasAudioProgrammeLanguage;
    int16_t hasMaxDuckingDepth;
    int16_t hasLoudnessMetadata;
    int16_t hasAudioProgrammeRefScreen;

    int16_t audioProgrammeLanguage;

    float maxDuckingDepth;

    LoudnessMetadata loudnessData;

    AudioProgrammeRefScreen audioProgrammeRefScreenData;

    int16_t numContents;

    int16_t refContentIdx[4]; 
}AudioProgramme, *AudioProgrammeHandle;

typedef struct Avs3BasicL1Structure {
    AudioProgramme audioProgrammeMeta;

    int16_t numOfContents;
    AudioContent audioContentData[4];

    int16_t numOfObjects;
    AudioObject audioObjectData[8];

    int16_t numOfPacks;
    AudioPackFormat audioPackFormatData[8];

    int16_t numOfChannels;
    AudioChannelFormat audioChannelFormatData[32];
}Avs3BasicL1, *Avs3BasicL1Handle;

typedef struct Avs3MetaDataStaticStructure {
    int16_t hasVrExt;
    int16_t basicLevel;

    Avs3BasicL1 avs3BasicL1;

    int16_t vrExtLevel;
    Avs3VrExtL1MetaData avs3VrExtL1MetaData;
}Avs3MetaDataStatic, *Avs3MetaDataStaticHandle;

// Dynamic
typedef struct Avs3DmL2MetaDataStructure {
    int16_t hasChannelLock;
    int16_t channelLock;
    float channelLockMaxDist;

    int16_t hasObjectDivergence;
    float objDivergence;
    float objDiverAzimuthRange;

    int16_t hasObjectScreenRef;
    int16_t objScreenRef;

    int16_t hasScreenEdgeLock;
    int16_t screenEdgeLock;
}Avs3DmL2MetaData, *Avs3DmL2MetaDataHandle;

typedef struct Avs3DmL1MetaDataStructure {
    int16_t cartesian;

    int16_t hasObjExtent;
    int16_t hasObjGain;
    int16_t hasObjDiffuse;
    int16_t hasObjImportance;

    float objAzimuth;
    float objElevation;
    float objDistance;
    float objWidth;
    float objHeight;
    float objDepth;

    float obj_x;
    float obj_y;
    float obj_z;
    float objWidth_x;
    float objHeight_y;
    float objDepth_z;

    float gain;
    float diffuse;
    int16_t jumpPosition;
    int16_t importance;
}Avs3DmL1MetaData, *Avs3DmL1MetaDataHandle;

typedef struct Avs3MetaDataDynamicStructure {
    int16_t dmLevel;
    int16_t numDmChans;
    int16_t muteFlag[32];
    int16_t transChRef[32];

    Avs3DmL1MetaData avs3DmL1MetaData[32];
    Avs3DmL2MetaData avs3DmL2MetaData[32];
}Avs3MetaDataDynamic, *Avs3MetaDataDynamicHandle;

// MasterMeta
typedef struct Avs3MetaDataStructure {
    int16_t hasStaticMeta;
    int16_t hasDynamicMeta;

    Avs3MetaDataStatic avs3MetaDataStatic;
    Avs3MetaDataDynamic avs3MetaDataDynamic;
}Avs3MetaData, *Avs3MetaDataHandle;

#endif
