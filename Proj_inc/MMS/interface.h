#ifndef _MMS_NEW_H_
#define _MMS_NEW_H_


#define MAX_PAGE_NUM                    50

#define UnrecognizedVersion (unsigned char)0x01

//the media type that we can support
/*
#define CT_TEXT_HTML "text/html"
#define CT_TEXT_PLAIN "text/plain"
#define CT_TEXT_WML "text/vnd.wap.wml"

#define CT_IMAGE_GIF "image/gif"
#define CT_IMAGE_JPEG "image/jpeg"
#define CT_IMAGE_TIFF "image/tiff"
#define CT_IMAGE_PNG "image/png"
#define CT_IMAGE_WBMP "image/vnd.wap.wbmp"
#define CT_IMAGE_BMP "image/bmp"

#define CT_AUDIO_WAV  "audio/wav"   
#define CT_AUDIO_MIDI  "audio/midi"
#define CT_AUDIO_MID   "audio/mid"
#define CT_AUDIO_GSM   "audio/gsm"
#define CT_AUDIO_MIDIM  "application/vnd.smaf"
#define CT_AUDIO_MP3  "audio/mp3"
#define CT_AUDIO_IMELODY "text/x-imelody"
#define CT_AUDIO_AMR "audio/amr"

*/
  /* Content Types Strings*/
#define CT_TEXT_HTML "text/html"
#define CT_TEXT_PLAIN "text/plain"
#define CT_TEXT_WML "text/vnd.wap.wml"

#define CT_IMAGE_GIF "image/gif"
#define CT_IMAGE_JPEG "image/jpeg"
#define CT_IMAGE_TIFF "image/tiff"
#define CT_IMAGE_PNG "image/png"
#define CT_IMAGE_WBMP "image/vnd.wap.wbmp"
#define CT_IMAGE_BMP "image/bmp"

#define CT_AUDIO_WAV  "audio/wav"   /*added by wanggang 2002/08/30*/
#define CT_AUDIO_XWAV  "audio/x-wav"   /*added by wanggang 2002/08/30*/
#define CT_AUDIO_MIDI  "audio/midi"
#define CT_AUDIO_MID  "audio/mid"
#define CT_AUDIO_GSM  "audio/gsm"
#define CT_AUDIO_MIDIM  "application/vnd.smaf"//"audio/mmf"//
#define CT_AUDIO_MP3  "audio/mp3"
#define CT_AUDIO_IMELODY "text/x-imelody"
#define CT_AUDIO_AMR "audio/amr"
typedef struct AfMMSMEDIAREGIONTag
{
   unsigned short  nTop;
   unsigned short  nLeft;
   unsigned short  nHight;
   unsigned short  nwidth;
   char            strFit[20];
} REDIAREGION;

typedef struct AfMMSPAGETag
{
      //page during
      char             dur[20];

      //content order
      char             order[4];

      //text information
      char             *pTextString;
      char             *strTextName;
      int              nTextLen;
      char             *textbegin;
      char             *textend;
      REDIAREGION      TextRegion;
      char            *TextType;

      //image information
      char             *pImageData;
      char             *strImageName;
      int              nImageSize;
      char             *imagebegin;
      char             *imageend;
      REDIAREGION      ImageRegion;
      char             *ImageType;

      //midi information
      char             *pMidiData;
      char             *strMidiName;
      int              nMidiSize;
      char             *midibegin;
      char             *midiend;
      char             *MidiType;

      char             *pUnknowMedia;
      char             *strMediaName;
      int              nMediaSize;
      char             *mediabegin;
      char             *mediaend;
      char             *MediaType;
} MMSPAGE;

typedef struct AfMMSEDITTag
{
      char           *pCnfId;   
      char           *pToAddr;  /*recipients's address*/
      char           *pCcAddr;  /*recipients's copy address*/
      char           *pSubject;
      unsigned short nSubjectLen;
      char           *pFrom;
      long           nDate;
      int            nPageNum;
      unsigned short ScreenWidth;
      unsigned short ScreenHeigth;
      MMSPAGE        MmsPage[MAX_PAGE_NUM];
} MMSEDIT;

typedef struct AFMMSSETTINGTag
{
     long              nExpiry;
     unsigned short    nDeliveryReport;
     unsigned short    nReadReport;
     unsigned short    nPrority;
     unsigned short    nSenderVisibility;   
     unsigned short    nUnsigned;   
} MMSSET;

typedef struct BUFFDATA{
	long length;
	char *data;
} MMSBUFFER;


#endif
