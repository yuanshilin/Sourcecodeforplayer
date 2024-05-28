
#ifndef _AVS3_CONST_META_H_
#define _AVS3_CONST_META_H_

// vrEXT number bits
#define NBITS_VR_AMBIS_ORDER        3   // 3bits  ambisonicOrder
#define NBITS_VR_REVERBTYPE         2   // 2bits  reverbType
#define NBITS_VR_EARLY_REFGAIN      7   // 7bits  earlyReflectionGain
#define NBITS_VR_LATE_REVGAIN       7   // 7bits  lateReverbGain
#define NBITS_VR_CONV_REVTYPE       5   // 5bits  convolutionReverbType
#define NBITS_VR_NUMSURFACE         3   // 3bits  numSurface
#define NBITS_VR_MARTERIAL          5   // 5bits  material
#define NBITS_VR_ABSORPTION         7   // 7bits  absorption
#define NBITS_VR_SCATTERING         7   // 7bits  scattering
#define NBITS_VR_NUM_VER            5   // 5bits  numVertices
#define NBITS_VR_VER_X              7   // 7bits  x
#define NBITS_VR_VER_Y              7   // 7bits  y
#define NBITS_VR_VER_Z              7   // 7bits  z
#define NBITS_VR_HRTFTYPE           4   // 4bits  hrtfType
#define NBITS_VR_HPTYPE             7   // 7bits  headphoneType[i]
#define NBITS_VR_EFFCHAIN           3   // 3bits  effectChain
#define NBITS_VR_NUMEQBAND          4   // 4bits  numEqband
#define NBITS_VR_EQTYPE             3   // 3bits  eqType
#define NBITS_VR_EQFC               7   // 7bits  eqFC
#define NBITS_VR_EQQ                6   // 7-1bits  eqQ
#define NBITS_VR_EQGAIN             7   // 7bits  eqGain
#define NBITS_VR_ATT_TIME           4   // 4bits   attackTime
#define NBITS_VR_REL_TIME           4   // 4bits   releaseTime
#define NBITS_VR_THRESHOLD          7   // 7bits  threshold
#define NBITS_VR_PREGAIN            7   // 7bits  preGain
#define NBITS_VR_POSTGAIN           7   // 7bits  postGain
#define NBITS_VR_RATIO              7   // 7bits  ratio
#define NBITS_VR_EFFGAIN            7   // 7bits  effectGain

// ADM bits
#define NBITS_META_FLAG             1    // 1bit, flag
#define NBITS_BASICLEVEL            3    // 3bit, basicLevel
#define NBITS_VREXTLEVEL            3    // 3bit, vrExtLevel
#define NBITS_AUDIO_PL              4    // 4bits, audioProgrammeLanguage
#define NBITS_MAX_DUCK              5    // 5bits, maxDuckingDepth
#define NBITS_LOUDNESS              5    // 5bits, integratedLoudness
#define NBITS_LOUDNESS_RANGE        5    // 5bits, loudnessRange
#define NBITS_TURE_PEAK             5    // 5bits, maxTruePeak
#define NBITS_MAX_MOMNETARY         5    // 5bits, maxMomentary
#define NBITS_SHORT_TERM            5    // 5bits, maxShortTerm
#define NBITS_DL_LOUNDESS           5    // 5bits, maxShortTerm
#define NBITS_ASPECT_RATIO          3    // 3bits, aspectRatio
#define NBITS_SCREEN_AZIMUTH        8    // 8bits, screenCentrePosition_azimuth
#define NBITS_SCREEN_ELEVATION      6    // 6bits, screenCentrePosition_elevation
#define NBITS_SCREEN_DISTANCE       4    // 4bits, screenCentrePosition_distance
#define NBITS_SCREEN_POLAR          7    // 7bits, screenWidth_polar
#define NBITS_SCREEN_X              8    // 8bits, screenCentrePosition_X
#define NBITS_SCREEN_Y              6    // 6bits, screenCentrePosition_Y
#define NBITS_SCREEN_Z              4    // 4bits, screenCentrePosition_Z
#define NBITS_SCREEN_CARTESIAN      7    // 7bits, screenWidth_cartesian
#define NBITS_NUMCONTENTS           2    // 2bits, numContents
#define NBITS_REF_CON_IDX           2    // 2bits, refcontentIdx
#define NBITS_CON_IDX               2    // 2bits, contentIdx
#define NBITS_CONTENT_LANGUAGE      4    // 4bits, audioContentLanguage
#define NBITS_DIA_ATTRIBUTE         2    // 2bits, dialogueAttribute
#define NBITS_DIA_TYPE              3    // 3bits, dialogueType
#define NBITS_NUM_GROUPS            2    // 2bits, numComplementaryObjectGroup
#define NBITS_NUM_COM_OBJS          3    // 3bits, numComplementaryObject
#define NBITS_COM_OBJ_IDX           3    // 3bits, ComplementaryObjectIdx
#define NBITS_NUM_OBJECTS           3    // 3bits, numObjects
#define NBITS_REF_OBJ_IDX           3    // 3bits, refObjectsidx
#define NBITS_OBJ_IDX               3    // 3bits, Objectsidx
#define NBITS_OBJ_LANGUAGE          4    // 4bits, audioObjectLanguage
#define NBITS_OBJ_IMPORTANCE        4    // 4bits, importance
#define NBITS_OBJ_NAME              8    // 8bits, ObjectName
#define NBITS_OBJ_GAINUNIT          1    // 1bit, gainUnit
#define NBITS_OBJ_HALFGAIN          6    // 7-1 bits, gain
#define NBITS_NUM_PACKS             3    // 3bits, numPacks
#define NBITS_REF_PACKS_IDX         3    // 3bits, refPackFormatIdx
#define NBITS_PACKS_IDX             3    // 3bits, PackFormatIdx
#define NBITS_GAIN_RANGEMIN         7    // 7bits, gainInteractionRange_min
#define NBITS_GAIN_RANGEMAX         7    // 7bits, gainInteractionRange_max
#define NBITS_INTERACT_RANGE_XMIN   8    // 8bits, positionInteractionRange_Xmin / positionInteractionRange_azimuthMin
#define NBITS_INTERACT_RANGE_XMAX   8    // 8bits, positionInteractionRange_Xmax / positionInteractionRange_azimuthMax
#define NBITS_INTERACT_RANGE_YMIN   6    // 6bits, positionInteractionRange_Ymin / positionInteractionRange_elevationMin
#define NBITS_INTERACT_RANGE_YMAX   6    // 6bits, positionInteractionRange_Ymax / positionInteractionRange_elevationMax
#define NBITS_INTERACT_RANGE_ZMIN   4    // 4bits, positionInteractionRange_Zmin / positionInteractionRange_distanceMin
#define NBITS_INTERACT_RANGE_ZMAX   4    // 4bits, positionInteractionRange_Zmax / positionInteractionRange_distanceMax
#define NBITS_PACK_IMPORTANCE       4    // 4bits, pack importance
#define NBITS_PACK_LABEL            3    // 3bits, pack label
#define NBITS_PACK_ABSDIS           5    // 5bits, absoluteDistance
#define NBITS_NORMALIZATION         2    // 2bits, normalization
#define NBITS_NCFRDFDIST            4    // 4bits, nfcRefDist
#define NBITS_SCREENREF             1    // 1bits, screenRef
#define NBITS_HOAORDER              3    // 3bits, hoaorder
#define NBITS_PACKFORMATID          6    // 6bits, packFormatID
#define NBITS_NUM_MATRIXCH          5    // 5bits, numMatrixOutputChannel
#define NBITS_PACKFORMAT_IDX        5    // 5bits, packFormatStartIdx
#define NBITS_NUM_CHS               5    // 5bits, numChannels
#define NBITS_REF_CH_IDX            5    // 5bits, refChannelidx
#define NBITS_DIRECTSPEAKER_AZI     8    // 8bits, azimuth
#define NBITS_DIRECTSPEAKER_ELE     6    // 6bits, elevation
#define NBITS_DIRECTSPEAKER_DIS     4    // 4bits, distance
#define NBITS_SCREEN_EDGELOCK       2    // 2bits, screenEdgeLock
#define NBITS_CH_FORMAT_IDX         5    // 5bit, channelFormatIdx
#define NBITS_HALF_CHGAIN           6    // 7-1bit, channelGain
#define NBITS_TRANS_CH_REF          5    // 5bit, chRef
#define NBITS_MATRIX_COEF           8    // 8bit, MatrixCoef[i]

// Dynamic
#define NBITS_DMLEVEL               3    // 3bits, dmLevel
#define NBITS_DM_TRANSCHREF         5    // 3bits, transChRef

// vrEXT resolution
#define RES_VREXT_VERTEX_Z          (200.0f / ((1 << 7) - 1))       // 7bit, range [-100, 100]
#define RES_VREXT_VERTEX_Y          (200.0f / ((1 << 7) - 1))       // 7bit, range [-100, 100]
#define RES_VREXT_VERTEX_X          (200.0f / ((1 << 7) - 1))       // 7bit, range [-100, 100]
#define RES_VREXT_ABSORPTION        (1.0f / ((1 << 7) - 1))         // 7bit, range [0, 1]
#define RES_VREXT_SCATTERING        (1.0f / ((1 << 7) - 1))         // 7bit, range [0, 1]
#define RES_VREXT_EAR_REF_GAIN      (1.0f / ((1 << 7) - 1))         // 7bit, range [0, 1]
#define RES_VREXT_LATE_REV_GAIN     (1.0f / ((1 << 7) - 1))         // 7bit, range [0, 1]
#define RES_VREXT_EQFC              (58.061799f / ((1 << 7) - 1))   // 7bit, range [20, 16000] convert to db [26.020599, 84.0823996]
#define RES_VREXT_EQQ_H1            (0.9f / ((1 << 6) - 1))         // 7-1bit, range [0.1, 1]
#define RES_VREXT_EQQ_H2            (11.0f / ((1 << 6) - 1))        // 7-1bit, range [1, 12]
#define RES_VREXT_EQGAIN            (40.0f / ((1 << 7) - 1))        // 7bit, range [-20, 20]
#define RES_VREXT_ATT_TIME          (99.0f / ((1 << 4) - 1))        // 4bit, range [1, 100]
#define RES_VREXT_REL_TIME          (250.0f / ((1 << 4) - 1))       // 4bit, range [50, 300]
#define RES_VREXT_THRESHOLD         (90.0f / ((1 << 7) - 1))        // 7bit, range [-80, 10]
#define RES_VREXT_PREGAIN           (20.0f / ((1 << 7) - 1))        // 7bit, range [-10, 10]
#define RES_VREXT_POSTGAIN          (20.0f / ((1 << 7) - 1))        // 7bit, range [0, 20]
#define RES_VREXT_RATIO             (99.0f / ((1 << 7) - 1))        // 7bit, range [1, 100]
#define RES_VREXT_EFFGAIN           (40.0f / ((1 << 7) - 1))        // 7bit, range [-20, 20]

// ADM resolution
#define RES_STATIC_RANGE_MIN            (1.0f / ((1 << 7) - 1))         // 7bit, range [0, 1]
#define RES_STATIC_RANGE_MAX            (15.0f / ((1 << 7) - 1))        // 7bit, range (1, 16]
#define RES_STATIC_RANGEDB_MIN          (-80.0f / ((1 << 7) - 1))       // 7bit, range [-80, 0]
#define RES_STATIC_RANGEDB_MAX          (24.0f / ((1 << 7) - 1))        // 7bit, range (0, 24]
#define RES_MTRIXCOEF                   (9.9f / ((1 << 8) - 1))         // 8bit, range [0.1, 10]
#define RES_DUCKINGDEPTH                (-62.0f / ((1 << 5) - 1))       // 5bit, range [-62, 0]
#define RES_STATIC_ABSDISTANCE          (1.230448f / ((1 << 5) - 1))    // 5bits range[0, 16]  convert to dB[0, 1.230448]
#define RES_STATIC_NFCREFDIST           (1.0f / ((1 << 4) - 1))         // 4bits range[0, 1]
#define RES_STATIC_OBJ_GAIN_H1          (1.0f / ((1 << 6) - 1))         // 6bit, range [0, 1]
#define RES_STATIC_OBJ_GAIN_H2          (15.0f / ((1 << 6) - 1))        // 6bit, range (1, 16]
#define RES_STATIC_OBJ_GAIN_DB_H1       (-80.0f / ((1 << 6) - 1))       // 6bit, range [-80, 0]
#define RES_STATIC_OBJ_GAIN_DB_H2       (24.0f / ((1 << 6) - 1))        // 6bit, range (0, 24]
#define RES_STATIC_OBJ_CHGAIN_H1        (1.0f / ((1 << 6) - 1))         // 6bit, range [0, 1]
#define RES_STATIC_OBJ_CHGAIN_H2        (15.0f / ((1 << 6) - 1))        // 6bit, range (1, 16]
#define RES_STATIC_OBJ_CHGAIN_DB_H1     (-80.0f / ((1 << 6) - 1))       // 6bit, range [-80, 0]
#define RES_STATIC_OBJ_CHGAIN_DB_H2     (24.0f / ((1 << 6) - 1))        // 6bit, range (0, 24]
#define RES_STATIC_X                    (2.0f / ((1 << 8) - 1))         // 8bit, range [-1, 1]
#define RES_STATIC_Y                    (2.0f / ((1 << 6) - 1))         // 6bit, range [-1, 1]
#define RES_STATIC_Z                    (2.0f / ((1 << 4) - 1))         // 4bit, range [-1, 1]
#define RES_STATIC_CARTWIDTH            (1.0f / ((1 << 7) - 1))         // 7bit, range [0.0, 1]

#define RES_STATIC_AZIMUTHMIN           (-180.0f / ((1 << 8) - 1))      // 8bit, range [-180, 0]
#define RES_STATIC_AZIMUTHMAX           (180.0f / ((1 << 8) - 1))       // 8bit, range (0, 180]
#define RES_STATIC_ELEVATIONMIN         (-90.0f / ((1 << 6) - 1))       // 6bit, range [-90, 0]
#define RES_STATIC_ELEVATIONMAX         (90.0f / ((1 << 6) - 1))        // 6bit, range (0, 90]
#define RES_STATIC_AZIMUTH              (360.0f / ((1 << 8) - 1))       // 8bit, range [-180, 180]
#define RES_STATIC_ELEVATION            (180.0f / ((1 << 6) - 1))       // 6bit, range [-90, 90]
#define RES_STATIC_SCREEELEVATION       (90.0f / ((1 << 6) - 1))        // 6bit, range [0, 90]
#define RES_STATIC_DISTANCE             (1.0f / ((1 << 4) - 1))         // 4bit, range [0.0, 1.0]
#define RES_STATIC_POLARWIDTH           (180.0f / ((1 << 7) - 1))       // 7bit, range [0.0, 180]

#define RES_STATIC_INTEGRATEDLOUDNESS   (-70.0f / ((1 << 5) - 1))       // 5bit, range [-70, 0]
#define RES_STATIC_LOUDNESSRANGE        (60.0f / ((1 << 5) - 1))        // 5bit, range [10, 70]
#define RES_STATIC_MAXTRUEPEAK          (-70.0f / ((1 << 5) - 1))       // 5bit, range [-70, 0]
#define RES_STATIC_MAXMOMENTARY         (-70.0f / ((1 << 5) - 1))       // 5bit, range [-70, 0]
#define RES_STATIC_MAXSHORTTERM         (-70.0f / ((1 << 5) - 1))       // 5bit, range [-70, 0]
#define RES_STATIC_DIALOGUELOUDNESS     (-70.0f / ((1 << 5) - 1))       // 5bit, range [-70, 0]

/* Metadata Dynamic */
// Level 1
#define RES_OBJ_AZIMUTH     (360.0f / ((1 << 8) - 1))       // 8bit, range [-180, 180]
#define RES_OBJ_ELEVATION   (180.0f / ((1 << 6) - 1))       // 6bit, range [-90, 90]
#define RES_OBJ_DISTANCE    (1.0f / ((1 << 4) - 1))         // 4bit, range [0.0, 1.0]
#define RES_OBJ_GAIN        (6.0f / ((1 << 7) - 1))         // 7bit, range [0.0, 6.0]
#define RES_OBJ_WIDTH       (360.0f / ((1 << 7) - 1))       // 7bit, range [0, 360]
#define RES_OBJ_HEIGHT      (360.0f / ((1 << 5) - 1))       // 5bit, range [0, 360]
#define RES_OBJ_DEPTH       (1.0f / ((1 << 4) - 1))         // 4bit, range [0.0, 1.0]
#define RES_OBJ_DIFFUSE     (1.0f / ((1 << 7) - 1))         // 7bit, range [0.0, 1.0]
#define RES_OBJ_X           (2.0f / ((1 << 8) - 1))         // 8bit, range [-1.0, 1.0]
#define RES_OBJ_Y           (2.0f / ((1 << 6) - 1))         // 6bit, range [-1.0, 1.0]
#define RES_OBJ_Z           (2.0f / ((1 << 4) - 1))         // 4bit, range [-1.0, 1.0]
#define RES_OBJ_WIDTH_X     (1.0f / ((1 << 7) - 1))         // 7bit, range [0, 1.0]
#define RES_OBJ_HEIGHT_Y    (1.0f / ((1 << 5) - 1))         // 5bit, range [0, 1.0]
#define RES_OBJ_DEPTH_Z     (1.0f / ((1 << 4) - 1))         // 4bit, range [0, 1.0]
// Level 2
#define RES_CHANNELLOCK_MAXDIST         (2.0f / ((1 << 4) - 1))     // 4bit, range [0.0, 2.0]
#define RES_OBJ_DIVERGENCE              (1.0f / ((1 << 4) - 1))     // 4bit, range [0.0, 1.0]
#define RES_OBJ_DIVERGENCE_AZI_RANGE    (180.0f / ((1 << 6) - 1))   // 6bit, range [0, 180]

/* Metadata num bits */
// Level 1
#define NBITS_OBJ_AZIMUTH           8           // nbits for azimuth
#define NBITS_OBJ_ELEVATION         6           // nbits for elevation
#define NBITS_OBJ_DISTANCE          4           // nbits for distance
#define NBITS_OBJ_GAIN              7           // nbits for obj gain
#define NBITS_OBJ_WIDTH             7           // nbits for obj width
#define NBITS_OBJ_HEIGHT            5           // nbits for obj height
#define NBITS_OBJ_DEPTH             4           // nbits for obj depth
#define NBITS_OBJ_DIFFUSE           7           // nbits for obj diffuse
#define NBITS_JUMPPOSITION          1           // nbits for jump position
#define NBITS_IMPORTANCE            4           // nbits for obj importance
#define NBITS_OBJ_X                 8           // nbits for x
#define NBITS_OBJ_Y                 6           // nbits for y
#define NBITS_OBJ_Z                 4           // nbits for z
#define NBITS_OBJ_WIDTH_X           7           // nbits for obj width x
#define NBITS_OBJ_HEIGHT_Y          5           // nbits for obj height y
#define NBITS_OBJ_DEPTH_Z           4           // nbits for obj depth z
// Level 2
#define NBITS_CHANNEL_LOCK                  1   // nbits for channel lock
#define NBITS_CHANNELLOCK_MAXDIST           4   // nbits for channel lock, max distance
#define NBITS_OBJ_DIVERGENCE                4   // nbits for object divergence
#define NBITS_OBJ_DIVERGENCE_AZI_RANGE      6   // nbits for object divergence, azimuth range
#define NBITS_OBJ_SCREEN_REF                1   // nbits for screen reference
#define NBITS_OBJ_SCREEN_EDGE_LOCK          2   // nbits for screen edge lock

#endif
