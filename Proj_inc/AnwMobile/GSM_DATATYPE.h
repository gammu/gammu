#if !defined(AFX_GSM_DATATYPE_H__6F6A37B9_8B86_4DDF_B7D8_FD5225F0466B__INCLUDED_)
#define AFX_GSM_DATATYPE_H__6F6A37B9_8B86_4DDF_B7D8_FD5225F0466B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GSM_DATATYPE.h : header file
//

#include "..\MbGlobals.h"
//#include "GSM_DATATYPE.h"
//All structure
#define  _MAX_NUMBER   32
class node
{
public:

    int Index;
	int Location;
	bool flag;
    node* link;
};

typedef struct
{
	int Phone_Type;
	char Number[_MAX_NUMBER];
}PHONE_NUMBER;
typedef struct
{
	int Phone_Type;
	int Boundary;
}PHONE_BOUNDARY;

typedef struct {
	// Used memory for phonebook entry
	GSM_MemoryType	  	MemoryType;

	int Total_Number;

	// Number of SubEntries in Entries table.
	int		     	EntriesNum;

	// Values of SubEntries.
	PHONE_BOUNDARY      Entries[GSM_PHONEBOOK_ENTRIES];
} GSM_Phone_Support_Entry;



typedef struct
{
   // HighByte : size limitation, LowByte : Mobile On=0x01, SIM On=0x02
   short Name; 
   // HighByte : size limitation, LowByte : Mobile On=0x01, SIM On=0x02
   short Address;
   // Mobile On=0x01, SIM On=0x02
   char Name_Photo;

} MobileContactStruct;

typedef struct
{
  // The numeric of SIM
  short SIMContactTotalNum;
  // -1 denode N/A:Can't getDriver
  short SIMContactUsedNum;
  // The numeric of ME
  short MEContactTatalNum;
  // -1 denode N/A:Can't getDriver
  short MEContactUsedNum;

} Contact_Tal_Num;

typedef enum {
	GSMFile_Java_JAR = 1,
	GSMFile_Image_JPG,
	GSMFile_Image_BMP,
	GSMFile_Image_GIF,
	GSMFile_Image_PNG,
	GSMFile_Image_WBMP,
    GSMFile_Video_3GP,
    GSMFile_Sound_AMR,
	GSMFile_Sound_NRT,		/* DCT4 binary format   */
    GSMFile_Sound_MIDI,
//#ifdef DEVELOP
	GSMFile_MMS,
//#endif
	GSMFile_Other
} GSMFileType;







/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GSM_DATATYPE_H__6F6A37B9_8B86_4DDF_B7D8_FD5225F0466B__INCLUDED_)
