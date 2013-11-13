#pragma once

class Tconfig;
class TffdshowParamInfo;
class TglobalSettingsBase;
class Tlibavcodec;
class Tlibmplayer;
class Ttranslate;

interface IffDShowBase : public IUnknown
{
    STDMETHOD_(int,getVersion2)(void) PURE;
    STDMETHOD (getParam)(unsigned int paramID, int* value) PURE;
    STDMETHOD_(int,getParam2)(unsigned int paramID) PURE;
    STDMETHOD (putParam)(unsigned int paramID, int  value) PURE;
    STDMETHOD (invParam)(unsigned int paramID) PURE;
    STDMETHOD (getParamStr)(unsigned int paramID,TCHAR *buf,size_t buflen) PURE;
    STDMETHOD_(const TCHAR*,getParamStr2)(unsigned int paramID) PURE; //returns const pointer to string, NULL if fail
    STDMETHOD (putParamStr)(unsigned int paramID,const TCHAR *buf) PURE;
    STDMETHOD (getParamName)(unsigned int i,TCHAR *buf,size_t len) PURE;
    STDMETHOD (notifyParamsChanged)(void) PURE;
    STDMETHOD (setOnChangeMsg)(HWND wnd,unsigned int msg) PURE;
    STDMETHOD (setOnFrameMsg)(HWND wnd,unsigned int msg) PURE;
    STDMETHOD (getGlobalSettings)(TglobalSettingsBase* *globalSettingsPtr) PURE;
    STDMETHOD (saveGlobalSettings)(void) PURE;
    STDMETHOD (loadGlobalSettings)(void) PURE;
    STDMETHOD (saveDialogSettings)(void) PURE;
    STDMETHOD (loadDialogSettings)(void) PURE;
    STDMETHOD (getConfig)(const Tconfig* *configPtr) PURE;
    STDMETHOD (getInstance)(HINSTANCE *hi) PURE;
    STDMETHOD_(HINSTANCE,getInstance2)(void) PURE;
    STDMETHOD (getPostproc)(Tlibmplayer* *postprocPtr) PURE;
    STDMETHOD (getTranslator)(Ttranslate* *trans) PURE;
    STDMETHOD (initDialog)(void) PURE;
    STDMETHOD (showCfgDlg)(HWND owner) PURE;
    STDMETHOD_(int,getCpuUsage2)(void) PURE;
    STDMETHOD_(int,getCpuUsageForPP)(void) PURE;
    STDMETHOD (cpuSupportsMMX)(void) PURE;
    STDMETHOD (cpuSupportsMMXEXT)(void) PURE;
    STDMETHOD (cpuSupportsSSE)(void) PURE;
    STDMETHOD (cpuSupportsSSE2)(void) PURE;
    STDMETHOD (cpuSupports3DNOW)(void) PURE;
    STDMETHOD (cpuSupports3DNOWEXT)(void) PURE;
    STDMETHOD (dbgInit)(void) PURE;
    STDMETHOD (dbgError)(const TCHAR *fmt,...) PURE;
    STDMETHOD (dbgWrite)(const TCHAR *fmt,...) PURE;
    STDMETHOD (dbgDone)(void) PURE;
    STDMETHOD (showTrayIcon)(void) PURE;
    STDMETHOD (hideTrayIcon)(void) PURE;
    STDMETHOD_(const TCHAR*,getExeflnm)(void) PURE;
    STDMETHOD (getLibavcodec)(Tlibavcodec* *libavcodecPtr) PURE;
    STDMETHOD_(const TCHAR*,getSourceName)(void) PURE;
    STDMETHOD (getGraph)(IFilterGraph* *graphPtr) PURE;
    STDMETHOD (seek)(int seconds) PURE;
    STDMETHOD (tell)(int*seconds) PURE;
    STDMETHOD (stop)(void) PURE;
    STDMETHOD (run)(void) PURE;
    STDMETHOD_(int,getState2)(void) PURE;
    STDMETHOD_(int,getCurTime2)(void) PURE;
    STDMETHOD (getParamStr3)(unsigned int paramID,const TCHAR* *bufPtr) PURE;
    STDMETHOD (savePresetMem)(void *buf,size_t len) PURE; //if len=0, then buf should point to int variable which will be filled with required buffer length
    STDMETHOD (loadPresetMem)(const void *buf,size_t len) PURE;
    STDMETHOD (getParamName3)(unsigned int i,const TCHAR* *namePtr) PURE;
    STDMETHOD (getInCodecString)(TCHAR *buf,size_t buflen) PURE;
    STDMETHOD (getOutCodecString)(TCHAR *buf,size_t buflen) PURE;
    STDMETHOD (getMerit)(DWORD *merit) PURE;
    STDMETHOD (setMerit)(DWORD  merit) PURE;
    STDMETHOD (lock)(int lockId) PURE;
    STDMETHOD (unlock)(int lockId) PURE;
    STDMETHOD (getParamInfo)(unsigned int i,TffdshowParamInfo *paramPtr) PURE;
    STDMETHOD (exportRegSettings)(int all,const TCHAR *regflnm,int unicode) PURE;
    STDMETHOD (checkInputConnect)(IPin *pin) PURE;
    STDMETHOD (getParamListItem)(int paramId,int index,const TCHAR* *ptr) PURE;
    STDMETHOD (abortPlayback)(HRESULT hr) PURE;
    STDMETHOD (notifyParam)(int id,int val) PURE;
    STDMETHOD (notifyParamStr)(int id,const TCHAR *val) PURE;
    STDMETHOD (doneDialog)(void) PURE;
    STDMETHOD (resetParam)(unsigned int paramID) PURE;
    STDMETHOD_(int,getCurrentCodecId2)(void) PURE;
    STDMETHOD (frameStep)(int diff) PURE;
    STDMETHOD (getInfoItem)(unsigned int index,int *id,const TCHAR* *name) PURE;
    STDMETHOD (getInfoItemValue)(int id,const TCHAR* *value,int *wasChange,int *splitline) PURE;
    STDMETHOD (inExplorer)(void) PURE;
    STDMETHOD_(const TCHAR*,getInfoItemName)(int id) PURE;
    STDMETHOD_(HWND,getCfgDlgHwnd)(void) PURE;
    STDMETHOD_(void,setCfgDlgHwnd)(HWND hwnd) PURE;
    STDMETHOD_(HWND,getTrayHwnd_)(void) PURE;
    STDMETHOD_(void,setTrayHwnd_)(HWND hwnd) PURE;
    STDMETHOD_(const TCHAR*,getInfoItemShortcut)(int id) PURE;
    STDMETHOD_(int,getInfoShortcutItem)(const TCHAR *s,int *toklen) PURE;
    STDMETHOD_(DWORD,CPUcount)(void) PURE;
};

const CLSID CLSID_FFDShowVideoDecoder = {0x04fe9017, 0xf873, 0x410e, { 0x87, 0x1e, 0xab, 0x91, 0x66, 0x1a, 0x4e, 0xf7}};
const CLSID CLSID_FFDShowAudioDecoder = {0x0f40e1e5, 0x4f79, 0x4988, { 0xb1, 0xa9, 0xcc, 0x98, 0x79, 0x4e, 0x6b, 0x55}};
const CLSID CLSID_FFDShowAudioProcessor = {0xb86f6bee, 0xe7c0, 0x4d03, { 0x8d, 0x52, 0x5b, 0x44, 0x30, 0xcf, 0x6c, 0x88}};
const IID IID_IffDShowBaseA = {0xec5bccf4, 0xfd62, 0x45ee, { 0xb0, 0x22, 0x38, 0x40, 0xea, 0xea, 0x77, 0xb2}};
const IID IID_IffDShowBaseW = {0xfc5bccf4, 0xfd62, 0x45ee, { 0xb0, 0x22, 0x38, 0x40, 0xea, 0xea, 0x77, 0xb2}};
const IID IID_IffDecoder = {0x00f99063, 0x70d5, 0x4bcc, { 0x9d, 0x88, 0x38, 0x01, 0xf3, 0xe3, 0x88, 0x1b}};

#ifdef UNICODE
#define IID_IffDShowBase IID_IffDShowBaseW
#else
#define IID_IffDShowBase IID_IffDShowBaseA
#endif


// Generic
#define IDFF_installPath          35

// Picture control
#define IDFF_isPictProp          205  // open or close the picture control
#define IDFF_lumGain             201  // luminance gain
#define IDFF_lumOffset           202  // luminance offset
#define IDFF_hue                 203  // hue 
#define IDFF_saturation          204  // saturation

// Video Resize
#define IDFF_filterResize        700
#define IDFF_showResize          751
#define IDFF_isResize            701 //is resizing active (or will be resizing active)
#define IDFF_orderResize         722
#define IDFF_fullResize          723
#define IDFF_resizeMode          728 //0 - exact size, 1 - aspect ratio , 2 - multiply of , 3 - multiply
#define IDFF_resizeDx            702 //new width
#define IDFF_is_resizeDy_0       703 //dummy for backward compatibility
#define IDFF_resizeDy            705 //new height
#define IDFF_resizeDy_real       766
#define IDFF_resizeMultOf        764
#define IDFF_resizeA1            729
#define IDFF_resizeA2            730
#define IDFF_resizeMult1000      753
#define IDFF_resizeIf            733 //0 - always, 1 - size, 2 - number of pixels
#define IDFF_resizeIfXcond       734 //-1 - less, 1 - more
#define IDFF_resizeIfXval        735 //width to be compared to
#define IDFF_resizeIfYcond       736 //-1 - less, 1 - more
#define IDFF_resizeIfYval        737 //height to be compared to
#define IDFF_resizeIfXYcond      738 //0 - and, 1 - or
#define IDFF_resizeIfPixCond     739 //-1 - less, 1 - more
#define IDFF_resizeIfPixVal      740
#define IDFF_bordersInside       755
#define IDFF_bordersUnits        756 //0 - percent, 1 - pixels
#define IDFF_bordersLocked       743
#define IDFF_bordersPercentX     741
#define IDFF_bordersX IDFF_bordersPercentX
#define IDFF_bordersPercentY     742
#define IDFF_bordersY IDFF_bordersPercentY
#define IDFF_bordersPixelsX      757
#define IDFF_bordersPixelsY      758

#define IDFF_isAspect            704 //0 - no aspect ratio correctio, 1 - keep original aspect, 2 - aspect ratio is set in IDFF_aspectRatio
#define IDFF_aspectRatio         707 //aspect ratio (<<16)

#define IDFF_resizeMethodLuma          706
#define IDFF_resizeMethodChroma        759
#define IDFF_resizeMethodsLocked       763
#define IDFF_resizeInterlaced          748 // 0 - progressive, 1 - interlaced, 2 - use picture type information
#define IDFF_resizeAccurateRounding    731
#define IDFF_resizeBicubicLumaParam    724
#define IDFF_resizeBicubicChromaParam  760
#define IDFF_resizeGaussLumaParam      726
#define IDFF_resizeGaussChromaParam    761
#define IDFF_resizeLanczosLumaParam    727
#define IDFF_resizeLanczosChromaParam  762
#define IDFF_resizeGblurLum            708 // *100
#define IDFF_resizeGblurChrom          709 // *100
#define IDFF_resizeSharpenLum          710 // *100
#define IDFF_resizeSharpenChrom        711 // *100
#define IDFF_resizeSimpleWarpXparam    749 // simple warped resize X param *1000
#define IDFF_resizeSimpleWarpYparam    750 // simple warped resize Y param *1000

// Volume control
#define IDFF_isVolume					2401 // open or close the volume control
#define IDFF_volume						2403 // volume

// DXVA setting
#define IDFF_filterMode						7
#define IDFF_FILTERMODE_PLAYER				1
#define IDFF_FILTERMODE_CONFIG				2
#define IDFF_FILTERMODE_PROC				4
#define IDFF_FILTERMODE_VFW					8
#define IDFF_FILTERMODE_VIDEO				256
#define IDFF_FILTERMODE_VIDEORAW			512
#define IDFF_FILTERMODE_AUDIO				1024
#define IDFF_FILTERMODE_ENC					2048
#define IDFF_FILTERMODE_AUDIORAW			4096
#define IDFF_FILTERMODE_VIDEOSUBTITLES		8192
#define IDFF_FILTERMODE_VIDEODXVA			16384

// DXVA
#define IDFF_dec_DXVA_H264					3536
#define IDFF_dec_DXVA_VC1					3537
#define IDFF_dec_DXVA_CompatibilityMode		3538
#define IDFF_dec_DXVA_PostProcessingMode	3539

// Subtiltes
#define IDFF_filterSubtitles         800
#define IDFF_isSubtitles             801
#define IDFF_showSubtitles           828
#define IDFF_orderSubtitles          815
#define IDFF_fullSubtitles           817
#define IDFF_subFilename             821
#define IDFF_subTempFilename        3402
#define IDFF_subPosX                 810
#define IDFF_subPosY                 811
#define IDFF_subAlign                827 //0 - old ffdshow mode, 1 - left, 2 - center, 3 - right
#define IDFF_subExpand               825 //0 - don't expand, 1 - 4:3, 2 - 16:9, xxxxyyyy - custom
#define IDFF_subIsExpand             858
#define IDFF_subDelay                812
#define IDFF_subSpeed                813
#define IDFF_subSpeed2               830
#define IDFF_subAutoFlnm             814
#define IDFF_subSearchDir            822
#define IDFF_subSearchExt            862
#define IDFF_subSearchHeuristic      856
#define IDFF_subWatch                826
#define IDFF_subEmbeddedPriority     3559
#define IDFF_subStereoscopic         833
#define IDFF_subStereoscopicPar      834 // stereoscopic parallax <-10%,10%> of picture width
#define IDFF_subDefLang              836
#define IDFF_subDefLang2             852
#define IDFF_subVobsub               835
#define IDFF_subVobsubAA             837
#define IDFF_subVobsubAAswgauss      851
#define IDFF_subVobsubChangePosition 849
#define IDFF_subImgScale             850
#define IDFF_subLinespacing          838
//#define IDFF_subTimeOverlap          839
#define IDFF_subIsMinDuration        840
#define IDFF_subMinDurationType      841 //0 - subtitle, 1 - line, 2 - character
#define IDFF_subMinDurationSub       842
#define IDFF_subMinDurationLine      843
#define IDFF_subMinDurationChar      844
#define IDFF_subTextpin              845
#define IDFF_subSSA                  861
#define IDFF_subPGS                  3545 // Enable bluray subtitles
#define IDFF_subFiles                3546 // Enable subtitle files
#define IDFF_subText                 3547 // Enable text subtitles

#define IDFF_subShowEmbedded         857 //id of displayed embedded subtitle, 0 if none
//#define IDFF_subFoundEmbedded        859
#define IDFF_subFix                  846
#define IDFF_subFixLang              847
#define IDFF_subFixDict              848
#define IDFF_subOpacity              853
#define IDFF_subSplitBorder          855
#define IDFF_subCC                   860
#define IDFF_subWordWrap             3403
#define IDFF_subExtendedTags         3492 // Enable HTML tags in SSA subs and SSA tags in SRT subs
#define IDFF_subSSAOverridePlacement 3496
#define IDFF_subSSAMaintainInside    3497
#define IDFF_subSSAUseMovieDimensions 3498

// Subtitles Font
#define IDFF_fontName                 820
#define IDFF_fontCharset              802
#define IDFF_fontAutosize             823
#define IDFF_fontAutosizeVideoWindow  829
#define IDFF_fontSizeP                803
//#define IDFF_fontSize               803
#define IDFF_fontSizeA                824
#define IDFF_fontWeight               804
#define IDFF_fontOutlineWidth         3387
#define IDFF_fontOpaqueBox            3400
#define IDFF_fontSpacing              808
#define IDFF_fontColor                809
#define IDFF_fontOutlineColor         3391
#define IDFF_fontShadowColor          3392
#define IDFF_fontSplitting            831
#define IDFF_fontXscale               832 // *100, multiplier of character width
#define IDFF_fontYscale               3411
#define IDFF_fontAspectAuto           3415
#define IDFF_fontFast                 854
#define IDFF_fontShadowMode           3374 // 0 - Glowing, 1 - classic gradient, 2 - classic
#define IDFF_fontBlurMode             3560
#define IDFF_fontShadowSize           3375
#define IDFF_fontBodyAlpha            3389
#define IDFF_fontOutlineAlpha         3390
#define IDFF_fontShadowAlpha          3376
#define IDFF_fontBlur                 3406 // 0 - disabled, 1 - enabled, 2 - enabled for border only
#define IDFF_fontShadowOverride       3548
#define IDFF_fontOutlineWidthOverride 3550
#define IDFF_fontSizeOverride         3551
#define IDFF_fontSettingsOverride     3552
#define IDFF_fontColorOverride        3553
#define IDFF_scaleBorderAndShadowOverride 3554
#define IDFF_fontItalic               3555
#define IDFF_OSDfontItalic            3556
#define IDFF_fontUnderline            3557
#define IDFF_OSDfontUnderline         3558

// Other constancts
#define IDFF_isWhitelist         3372
#define IDFF_trayIcon               3  // is tray icon visible

// Codecs
#define IDFF_xvid               1001 //are AVIs with this FOURCC played by ffdshow?
#define IDFF_div3               1002
#define IDFF_mp4v               1003
#define IDFF_dx50               1004
#define IDFF_fvfw               1022
#define IDFF_mp43               1005
#define IDFF_mp42               1006
#define IDFF_mp41               1007
#define IDFF_h263               1008
#define IDFF_h264               1047
#define IDFF_h261               1065
#define IDFF_wmv1               1011
#define IDFF_wmv2               1017
#define IDFF_wmv3               1042
#define IDFF_wvc1               1090
#define IDFF_cavs               1091
#define IDFF_vp5                1093
#define IDFF_vp6                1094
#define IDFF_vp6f               1095
#define IDFF_rt21               1096
#define IDFF_vixl               1097
#define IDFF_aasc               1098
#define IDFF_qtrpza             1099
#define IDFF_mjpg               1014
#define IDFF_dvsd               1015
#define IDFF_hfyu               1016
#define IDFF_cyuv               1019
#define IDFF_mpg1               1012
#define IDFF_mpg2               1013
#define IDFF_mpegAVI            1021
#define IDFF_asv1               1020
#define IDFF_vcr1               1040
#define IDFF_rle                1041
#define IDFF_theo               1023
#define IDFF_rv10               1037
#define IDFF_ffv1               1038
#define IDFF_vp3                1039
#define IDFF_tscc               1060
#define IDFF_rawv               1009 // 0 - unsupported, 1 - all, 2 - all YUV, 3 - all RGB, else FOURCC of accepted colorspace
#define IDFF_alternateUncompressed 3410 // Enable alternate method to enable FFDShow on uncompressed streams (for Vista VMP11 and VMC)
#define IDFF_isDyInterlaced     1330 // enable height dependant interlaced colorspace conversions
#define IDFF_dyInterlaced       1331
#define IDFF_svq1               1025
#define IDFF_svq3               1026
#define IDFF_cram               1027
#define IDFF_iv32               1034
#define IDFF_cvid               1035
#define IDFF_mszh               1044
#define IDFF_zlib               1045
#define IDFF_flv1               1049
#define IDFF_8bps               1050
#define IDFF_png1               1051
#define IDFF_qtrle              1052
#define IDFF_duck               1053
#define IDFF_qpeg               1064
#define IDFF_loco               1066
#define IDFF_wnv1               1067
#define IDFF_cscd               1072
#define IDFF_zmbv               1073
#define IDFF_ulti               1074
#define IDFF_wma7               1028
#define IDFF_wma8               1029
#define IDFF_mp2                1030
#define IDFF_mp3                1031
#define IDFF_ac3                1032
#define IDFF_eac3               1088
#define IDFF_dts                1057
#define IDFF_dtsinwav           1059
#define IDFF_aac                1033
#define IDFF_amr                1046
#define IDFF_iadpcm             1054
#define IDFF_msadpcm            1061
#define IDFF_otherAdpcm         1069
#define IDFF_law                1062
#define IDFF_gsm                1063
#define IDFF_flac               1055
#define IDFF_tta                1068
#define IDFF_qdm2               1070
#define IDFF_mace               1075
#define IDFF_truespeech         1071
#define IDFF_vorbis             1058
#define IDFF_lpcm               1056
#define IDFF_fps1               1077
#define IDFF_ra                 1079
#define IDFF_imc                1080
#define IDFF_mss2               1081
#define IDFF_wvp2               1082
#define IDFF_em2v               1083
#define IDFF_avrn               1084
#define IDFF_cdvc               1085
#define IDFF_atrac3             1086
#define IDFF_nellymoser         1087
#define IDFF_wavpack            1089 //1088 is eac3
#define IDFF_rawa               1036
#define IDFF_avisV              1043
#define IDFF_avisA              1048
#define IDFF_mlp                1097
#define IDFF_truehd             3515
#define IDFF_rv40               1332
#define IDFF_rv30               1333
#define IDFF_cook               1334
#define IDFF_vp8                1335
#define IDFF_iv50               1336
#define IDFF_i263               1337


#define IDFF_filterDeinterlace         1400
#define IDFF_isDeinterlace             1401
#define IDFF_showDeinterlace           1418
#define IDFF_orderDeinterlace          1424
#define IDFF_fullDeinterlace           1402
#define IDFF_deinterlaceAlways         3493
#define IDFF_swapFields                1409
#define IDFF_deinterlaceMethod         1403
#define IDFF_tomocompSE                1407
#define IDFF_tomocompVF                1414
#define IDFF_dscalerDIflnm             1412
#define IDFF_dscalerDIcfg              1413
#define IDFF_frameRateDoublerThreshold 1416
#define IDFF_frameRateDoublerSE        1417
#define IDFF_kernelDeintThreshold      1420
#define IDFF_kernelDeintSharp          1421
#define IDFF_kernelDeintTwoway         1422
#define IDFF_kernelDeintMap            1423
#define IDFF_kernelDeintLinked         1428
#define IDFF_dgbobMode                 1425
#define IDFF_dgbobThreshold            1426
#define IDFF_dgbobAP                   1427
#define IDFF_yadifMode                 3494
#define IDFF_yadifFieldOrder           3495
