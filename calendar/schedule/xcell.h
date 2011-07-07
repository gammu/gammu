#pragma once

#include "..\UnicodeString\UnicodeString.h"

//alignment define
static const int ALIGN_LEFT = DT_LEFT; //0
static const int ALIGN_CENTER = DT_CENTER; //
static const int ALIGN_RIGHT = DT_RIGHT; //

static const int ALIGN_TOP = DT_TOP; //0
static const int ALIGN_MIDDLE = DT_VCENTER; //
static const int ALIGN_BOTTOM = DT_BOTTOM; //

static const int ALIGN_MASK = ALIGN_LEFT | ALIGN_CENTER | ALIGN_RIGHT
							| ALIGN_TOP | ALIGN_MIDDLE | ALIGN_BOTTOM;

typedef enum _SCHEDULE_LOCALE {
	LOCALE_ENG = 0,
	LOCALE_TC = 1
} SCHEDULE_LOCALE;

#define HIGH_LIGHT_BG_COLOR RGB(0x44,0x44,0x99)
#define HIGH_LIGHT_TEXT_COLOR RGB(0xff,0xff,0xff)

class XCell
{
public:
	XCell(void);
	~XCell(void);
	
	XCell* operator = (XCell& cell);
	static CBitmap m_AlarmBmp;
	static CBitmap m_RepeatBmp;
	static CBitmap m_AlarmGrayBmp;
	static CBitmap m_RepeatGrayBmp;
	static CBitmap m_MoreBmp;
	static CBitmap m_MoreGrayBmp;
	static void SetAlarmBmp(int id);
	static void SetRepeatBmp(int id);
	static void SetAlarmGrayBmp(int id);
	static void SetRepeatGrayBmp(int id);
	static void SetMoreBmp(int id);
	static void SetMoreGrayBmp(int id);
	void SetTextLine(BOOL btl) {m_bTextLine = btl;}
	BOOL m_bTextLine;

public:
	int rowSpan;
	int colSpan;

	//	CString text;
//	CString text[48];
	CString text[48];
	BOOL m_bRepeat[48];
	BOOL m_bAlarm[48];
	int textCount;
	COLORREF textColor;
	COLORREF textlineColor;
	CFont* textFont;
	int textFontSize;
	CString textFontFace;


	CString label;
	COLORREF labelColor;
	CFont* labelFont;
	int labelFontSize;


	int format;
	int leftMargin;
	int rightMargin;

	int backMode;
	COLORREF backColor;

	COLORREF borderColor;
	int borderStyle;

	bool overlap;
	bool m_bDrawUpLine;
	bool m_bDrawDnLine;
	bool m_bDrawLeftLine;
	bool m_bDrawRightLine;

public:
	void Init();
	int SetSpan(int rows, int cols);

	int SetRepeat(BOOL bRepeat);
	int SetRepeat(int idx,BOOL bRepeat);
	int SetRepeat(BOOL bRepeat[48],int count);

	int SetAlarm(BOOL bAlarm);
	int SetAlarm(int idx,BOOL bAlarm);
	int SetAlarm(BOOL bAlarm[48],int count);

	int SetText(CString str);
	int SetText(int idx,CString str);
	void SetTextCount(int tc) { textCount = tc;}
	int SetText(CString strText[48],int strCount);
	CString GetText();
	CString GetText(int idx);
	int SetTextColor(COLORREF color);
	int SetTextLineColor(COLORREF color);
	COLORREF GetTextColor();
	int SetTextFont(CFont* font);
	CFont* GetTextFont();
	int SetTextFontSize(int size);
	int GetTextFontSize();

	int SetLabel(CString str);
	CString GetLabel();
	int SetLabelColor(COLORREF color);
	COLORREF GetLabelColor();
	int SetLabelFont(CFont* font);
	CFont* GetLabelFont();
	int SetLabelFontSize(int size);
	int GetLabelFontSize();

	int SetFormat(int format);
	int GetFormat();

	int SetLeftMargin(int pixels);
	int GetLeftMargin();

	int SetRightMargin(int pixels);
	int GetRightMargin();

	int SetBackMode(int mode);
	int GetBackMode();

	int SetBackColor(COLORREF color);
	COLORREF GetBackColor();

	int SetBorderSyle(int syle);
	int GetBorderSyle();

	int SetOverlap (bool enable);
	bool GetOverlap ();

	int SetAlignment (int align);
	int GetAlignment ();
	int SetSingleLine (bool enable);
	bool GetSingleLine ();

	int SetWordbreak (bool enable);
	bool GetWordbreak ();

	int SetEllipsis (bool enable);
	bool GetEllipsis ();
	
	int CalcTextRect (CDC* pDC, RECT* rect);

	void SetBorderColor(COLORREF color);
	int SetDrawUpLine(BOOL bDraw);
	int SetDrawDnLine(BOOL bDraw);
	int SetDrawRightLine(BOOL bDraw);
int SetDrawLeftLine(BOOL bDraw);

public:
	int DrawHighLight (CDC* pDC, RECT rect);
	void DrawBitmap(CDC *pDC, RECT rect, CBitmap &bmp);
	int Draw(CDC* pDC, RECT rect,BOOL bMore,RECT moreRect,CBitmap &image,BOOL bdrawBackground=TRUE);
	int Draw(CDC *pDC, RECT rect, int align, CBitmap &bmp);
	//Test, Nono_2004_0407
    int Draw2Bitmap(CDC *pDC, RECT rect, CBitmap &bmpL, CBitmap &bmpR);

	int Draw(CDC* pDC, RECT rect,BOOL bDrawBackground);
	int GetSpan(int& rows,int& cols);
	int Draw(CDC* pDC, RECT rect);
	int DrawText(CDC* pDC, RECT rect);
	int DrawLabel(CDC* pDC, RECT rect);
	int DrawBorder(CDC* pDC, RECT rect);
	int DrawBackground (CDC* pDC, RECT rect);
	int DrawHitBorder (CDC* pDC, RECT rect, COLORREF color); 


};
