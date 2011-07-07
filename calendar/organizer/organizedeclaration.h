#if !defined(AFX_ORGANIZERDECLARATION_H__F3DF63D9_6EAF_44BF_A3B0_AD9152E695BD__INCLUDED_)
#define AFX_ORGANIZERDECLARATION_H__F3DF63D9_6EAF_44BF_A3B0_AD9152E695BD__INCLUDED_
//Sanjeev
//#include "..\MobileDevice\CAMMSyncInclude.h"
#include "..\UnicodeString\UnicodeString.h"
#include "OleDateTimeEx.h"


#define Member_Seperator "Member_Sep_akfsda*(#$^#*@LKSJDFKSDJF#324324~!@#$^^(*$^@#*SLDJFLSDJF"
#define Object_Seperator "Object_Sep_akfsda*(#$^#*@LKSJDFKSDJF#324324~!@#$^^(*$^@#*SLDJFLSDJF"
/*
//Declaration for Phone book function.
typedef enum {
	CATEGORY_INIT = -1,
	CATEGORY_NORMAL = 0,
    CATEGORY_FRIEND = 1,
	CATEGORY_BUSINESS,
	CATEGORY_FAMILY,
	CATEGORY_VIP
} PhoneBookCategory;
#define MAX_PHB_FIELDS 11
typedef struct _PHONE_BOOK
{
    CString usFirstName;
    CString usLastName;
	//    PhoneBookCategory eCategory;
	CString usCategory;
    CString usCompanyName;
    CString usCellPhoneNo;
    CString usFamilyPhoneNo;
    CString usCompanyPhoneNo;
    CString usPagerNo;
    CString usFaxNo;
    CString usEmail;
    CString usMemo;
	GUID OrgId;
    CString usSound;
	CString usQQ;

	_PHONE_BOOK() {
		OrgId = IID_NONE;
		usSound = _T("");

		usFirstName = _T("");
		usLastName = _T("");
		//		eCategory = CATEGORY_NORMAL;
		usCategory = _T("");
		usCompanyName = _T("");
		usCellPhoneNo = _T("");
		usFamilyPhoneNo = _T("");
		usCompanyPhoneNo = _T("");
		usPagerNo = _T("");
		usFaxNo = _T("");
		usEmail = _T("");
		usMemo = _T("");
		usQQ = _T("");
	}
	CString GetClipboardString() {
		CString cpStr;
		CString sep = Member_Seperator;
		
		cpStr = sep;
		cpStr += usFirstName;

		cpStr += sep;
		cpStr += usLastName;
		
		cpStr += sep;
		cpStr += usCategory;

		cpStr += sep;
		cpStr += usCompanyName;
		
		cpStr += sep;
		cpStr += usCellPhoneNo;

		cpStr += sep;
		cpStr += usFamilyPhoneNo;
		
		cpStr += sep;
		cpStr += usCompanyPhoneNo;

		cpStr += sep;
		cpStr += usPagerNo;
		
		cpStr += sep;
		cpStr += usFaxNo;
		
		cpStr += sep;
		cpStr += usEmail;
		
		cpStr += sep;
		cpStr += usMemo;

		cpStr += sep;
		cpStr += usQQ;
		
		cpStr += sep;
		cpStr += usSound;

		cpStr += sep;
	
		return cpStr;
	}

	void SetPBValue(int idx,CString str) {
		switch (idx) {
		case 0:
			usFirstName = str;
			break;
		case 1:
			usLastName = str;
			break;
		case 2:
			usCategory = str;
			break;
		case 3:
			usCompanyName = str;
			break;
		case 4:
			usCellPhoneNo = str;
			break;
		case 5:
			usFamilyPhoneNo = str;
			break;
		case 6:
			usCompanyPhoneNo = str;
			break;
		case 7:
			usPagerNo = str;
			break;
		case 8:
			usFaxNo = str;
			break;
		case 9:
			usEmail = str;
			break;
		case 10:
			usMemo = str;
			break;
		case 11:
			usQQ = str;
			break;
		case 12:
			usSound = str;
			break;
	
		}
	}
	BOOL ParseClipboardString(CString cpStr) {
		CString sep = Member_Seperator;
		int pos=0,pos2=0;
		int idx=0;
		pos2 = cpStr.Find(sep.GetBuffer(),pos);
		if (pos2 < 0) {
			return FALSE;
		}
		pos2 += sep.GetLength();
		while (pos2 > 0) {
			pos = pos2;
			pos2 = cpStr.Find(sep.GetBuffer(),pos);
			SetPBValue(idx,cpStr.Mid(pos,pos2-pos));
			pos2 += sep.GetLength();
			if (pos2 >= cpStr.GetLength()) {
				break;
			}
			idx++;
			if (idx > MAX_PHB_FIELDS) {
				break;
			}
		}
		if (idx == MAX_PHB_FIELDS) {
			return TRUE;
		}
		return FALSE;
	}


	//	void SetData(_PHONE_BOOK &pb) {
	//		usFirstName = pb.usFirstName;
	//		usLastName = pb.usLastName;
	//		//		eCategory = pb.eCategory;
	//		usCategory = pb.usCategory;
	//		usCompanyName = pb.usCompanyName;
	//		usCellPhoneNo = pb.usCellPhoneNo;
	//		usFamilyPhoneNo = pb.usFamilyPhoneNo;
	//		usCompanyPhoneNo = pb.usCompanyPhoneNo;
	//		usPagerNo = pb.usPagerNo;
	//		usFaxNo = pb.usFaxNo;
	//		usEmail = pb.usEmail;
	//		usMemo = pb.usMemo;
	//		usSound = pb.usSound;
	//      OrgId = pb.OrgId;
	//	}

	BOOLEAN NotEqual(_PHONE_BOOK& pb) {
		if (usFirstName.GetLength() != pb.usFirstName.GetLength())	return TRUE;
		if (usFirstName.GetLength() != 0) {
			if (usFirstName != pb.usFirstName) return TRUE;
		}

		if (usLastName.GetLength() != pb.usLastName.GetLength())	return TRUE;
		if (usLastName.GetLength() != 0) {
			if (usLastName != pb.usLastName) return TRUE;
		}

		//		if (eCategory != pb.eCategory) 	return TRUE;
		if (usCategory.GetLength() != pb.usCategory.GetLength())	return TRUE;
		if (usCategory.GetLength() != 0) {
			if (usCategory != pb.usCategory) return TRUE;
		}

		if (usCompanyName.GetLength() != pb.usCompanyName.GetLength())	return TRUE;
		if (usCompanyName.GetLength() != 0) {
			if (usCompanyName != pb.usCompanyName) return TRUE;
		}

		if (usCellPhoneNo.GetLength() != pb.usCellPhoneNo.GetLength())	return TRUE;
		if (usCellPhoneNo.GetLength() != 0) {
			if (usCellPhoneNo != pb.usCellPhoneNo) return TRUE;
		}

		if (usFamilyPhoneNo.GetLength() != pb.usFamilyPhoneNo.GetLength())	return TRUE;
		if (usFamilyPhoneNo.GetLength() != 0) {
			if (usFamilyPhoneNo != pb.usFamilyPhoneNo) return TRUE;
		}

		if (usCompanyPhoneNo.GetLength() != pb.usCompanyPhoneNo.GetLength())	return TRUE;
		if (usCompanyPhoneNo.GetLength() != 0) {
			if (usCompanyPhoneNo != pb.usCompanyPhoneNo) return TRUE;
		}

		if (usPagerNo.GetLength() != pb.usPagerNo.GetLength())	return TRUE;
		if (usPagerNo.GetLength() != 0) {
			if (usPagerNo != pb.usPagerNo) return TRUE;
		}

		if (usFaxNo.GetLength() != pb.usFaxNo.GetLength())	return TRUE;
		if (usFaxNo.GetLength() != 0) {
			if (usFaxNo != pb.usFaxNo) return TRUE;
		}

		if (usEmail.GetLength() != pb.usEmail.GetLength())	return TRUE;
		if (usEmail.GetLength() != 0) {
			if (usEmail != pb.usEmail) return TRUE;
		}

		if (usMemo.GetLength() != pb.usMemo.GetLength())	return TRUE;
		if (usMemo.GetLength() != 0) {
			if (usMemo != pb.usMemo) return TRUE;
		}
		
		if (usQQ.GetLength() != pb.usQQ.GetLength())	return TRUE;
		if (usQQ.GetLength() != 0) {
			if (usQQ != pb.usQQ) return TRUE;
		}

		return FALSE;
	}
} PHONE_BOOK;
*/
typedef struct _MEMO
{
	CString usTitle;
	CString usMemo;
	GUID OrgId;

	_MEMO() {
		OrgId = IID_NONE;
		usTitle = _T("");
		usMemo = _T("");
	}
	CString GetClipboardString() {
		CString cpStr;
		CString sep = Member_Seperator;
		cpStr = sep;
		cpStr += usTitle;
		cpStr += sep;
		cpStr += usMemo;
		cpStr += sep;

		return cpStr;
	}

	void SetMemoValue(int idx,CString str) {
		switch (idx) {
		case 0:
			usTitle = str;
			break;
		case 1:
			usMemo = str;
			break;
		}
	}
	BOOL ParseClipboardString(CString cpStr) {
		CString sep = Member_Seperator;
		int pos=0,pos2=0;
		int idx=0;
		pos2 = cpStr.Find(sep,pos);
		if (pos2 < 0) {
			return FALSE;
		}
		pos2 += sep.GetLength();
		while (pos2 > 0) {
			pos = pos2;
			pos2 = cpStr.Find(sep,pos);
			SetMemoValue(idx,cpStr.Mid(pos,pos2-pos));
			pos2 += sep.GetLength();
			if (pos2 >= cpStr.GetLength()) {
				break;
			}
			idx++;
			if (idx > 1) {
				break;
			}
		}
		if (idx == 1) {
			return TRUE;
		}
		return FALSE;
	}
	BOOLEAN NotEqual(_MEMO& memo) {
		if (usTitle.GetLength() != memo.usTitle.GetLength())	return TRUE;
		if (usTitle.GetLength() != 0) {
			if (usTitle != memo.usTitle) return TRUE;
		}

		if (usMemo.GetLength() != memo.usMemo.GetLength())	return TRUE;
		if (usMemo.GetLength() != 0) {
			if (usMemo != memo.usMemo) return TRUE;
		}

		return FALSE;
	}
} OW_MEMO;

#define MAX_LENGTH 512
#define MAX_MEMO_LENGTH 2048

typedef enum {
	HALFDAY_AM = 0,
	HALFDAY_PM = 1
}HALF_DAY;

#define delimitor "@23#43@34#%34%"

typedef enum _Schedule_DatePeriod {
	DATEPERIOD_DAILY = 0,
	DATEPERIOD_WEEKLY = 1,
	DATEPERIOD_MONTHLY,
	DATEPERIOD_LIST
}Schedule_DatePeriod;


#define DEFAULT_CATEGORY_COUNT 5
#define MAX_CONTACTS_COUNT  2000
#define MAX_SCHEDULE_COUNT 1000
#define MAX_NOTES_COUNT  500

#define MAX_PLACE_NAME_LENGTH 3

#endif  //AFX_ORGANIZERDECLARATION_H__F3DF63D9_6EAF_44BF_A3B0_AD9152E695BD__INCLUDED_