#if !defined(AFX_COMMONDECLARATION_H__F3DF53D9_6EAF_44BF_A3B0_AD9152E695BD__INCLUDED_)
#define AFX_COMMONDECLARATION_H__F3DF53D9_6EAF_44BF_A3B0_AD9152E695BD__INCLUDED_
//Sanjeev
#include "MobileDevice\CAMMSyncInclude.h"

#define ET_SIZE 15  //Enhance Type enumeration size
#define MOBILE_INI "OkWapMobile.ini"

typedef enum {DEVICE_DISCONNECTED=-2,
			DEVICE_UNUSABLE=-1,
			DEVICE_UNKNOWN=0,
			DEVICE_CONNECTED=1,
			DEVICE_BUSY=2,
			DEVICE_INVALID=3} DeviceState;
typedef enum {dtNone,dtMelody, dtRing, dtMessage, dtLogo, dtPowerOnAvi, dtEmailSetting, \
		dtSetting, dtWAPSetting, dtMessageFolder, dtAddress, dtNotepad, dtCalender} MobileDataType;








typedef enum
{
        CAMM_ERROR_NONE               = 0,
        CAMM_ERROR_SUCCESS            = 1,
        CAMM_ERROR_FAILURE,            
        CAMM_ERROR_INVALIDFILE,
        CAMM_ERROR_UNKNOWN,            
        CAMM_ERROR_INITFAILED, 
        CAMM_ERROR_INVALIDFORMAT,
        CAMM_ERROR_EXCEPTION,  
        CAMM_ERROR_INVALIDARG, 
        CAMM_ERROR_FILE,                       
        CAMM_ERROR_FILESAVE,   
        CAMM_ERROR_FILEREAD,           
        CAMM_ERROR_FILEOPEN,           
        CAMM_ERROR_FILECLOSE,                  
        CAMM_ERROR_MEMORY,             
        CAMM_ERROR_OPERATIONNOTSUPPORTED,
        CAMM_ERROR_VALUEEXCEEDED,
        CAMM_ERROR_INVALIDOBJECT,
        CAMM_ERROR_INVALIDCONTEXT,
        CAMM_ERROR_FILEEXISTS  
} CAMMError;

typedef enum {
        EN_NONE = 0,
        EN_AUTO = 1,
        EN_TONE ,
		EN_TONE_BACKLIGHT,
		EN_TONE_OVEREXPOSURE,
		EN_TONE_UNDEREXPOSURE,
		EN_TONE_NIGHTSCENE,
		EN_TONE_SNOWSCENE,
        EN_SMART_CONTRAST,
        EN_COLORFUL,
        EN_COLORCAST,
        EN_VIVID,
        EN_BACKLIGHTING,
        EN_SKIN_LIGHTING,
		EN_SKIN_TONE
} EnhancementType;

typedef enum {
        PHOTO_LANDSCAPE = 0,
        PHOTO_PORTRAIT = 1,
        PHOTO_BACKLIGHT,
        PHOTO_COLORFUL,
        PHOTO_TEMPRATURE ,
        PHOTO_VIVID,
        PHOTO_AGED,
		PHOTO_COLORCAST,
		PHOTO_DISTOR,	
        PHOTO_TYPE_UNKNOWN
} PhotoType_OKWAP;


typedef struct _tagOutlookToOrganizerMap{
	int a;
}OutlookToOrganizerMap;


typedef enum {
	WHITEBOARD = 0,
	NEWSPAPER = 1,
	MAGAZINE,
	NAMECARD,
	DOCUMENT_UNKNOWN
}DocumentType;

typedef enum {
	SKINFLASH = 0,
	SKINCLEAR = 1,
	SKINCOLORFUL,
	BEAUTY_UNKNOWN
} BeautyType;

typedef struct _WRY {
	int WHITE;
	int RED;
	int YELLOW;
}WRY;

typedef enum {
	FISHEYE = 0,
	SWIRL = 1,
	SEPIA,
	SNOW,
	FINDEDGE,
	SALON,
	OILPAINT,
	COMIC,
	WATERCOLOR1,
	WATERCOLOR2,
	COLORINK1,
	COLORINK2,
	COLORINK3,
	INK1,
	SEASON_SPRING,
	SEASON_SUMMER,
	SEASON_AUTUMN,
	SEASON_WINTER,
	SKY_SUNNY,
	SKY_CLOUDY,
	SKY_EVENING,
	FOLIAGE_GRASS,
	FOLIAGE_WITHERED,
	FOLIAGE_LEAF,
	EFFECT_UNKNOWN
}EffectType;

//First version
//#define FACE_EXPRESSION_GOOD_LOOKER	1
//#define FACE_EXPRESSION_TRICKY		2
//#define FACE_EXPRESSION_UNLUCKY		3
//#define FACE_EXPRESSION_UGLY		4
//#define FACE_EXPRESSION_PIGGY_NOSE	5
//typedef enum {
//	FACE_GOOD_LOOKER = 1,
//	FACE_TRICKY = 2,
//	FACE_UNLUCKY,
//	FACE_UGLY,
//	FACE_PIGGY_NOSE,
//	FACE_UNKNOWN
//}FaceType;

//Second version
//#define FACE_EXPRESSION_SAD		1
//#define FACE_EXPRESSION_ANGRY	2
//#define FACE_EXPRESSION_FOX		3
//#define FACE_EXPRESSION_MONKEY	4
//#define FACE_EXPRESSION_PIGGY	5
typedef enum {
	FACE_SAD = 1,
	FACE_ANGRY = 2,
	FACE_FOX,
	FACE_MONKEY,
	FACE_PIGGY,
	FACE_UNKNOWN
}FaceType;


#endif  //AFX_COMMONDECLARATION_H__F3DF53D9_6EAF_44BF_A3B0_AD9152E695BD__INCLUDED_