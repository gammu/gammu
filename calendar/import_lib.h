#ifndef __ANW_IMPORT_LIB_H
#define __ANW_IMPORT_LIB_H

//DLL CONTRUCT
#include "gdiplusH.h"

#include "anwLib.h"
#include <vector>

#pragma comment( lib, "gdiplus.lib")

#ifdef _UNICODE
#pragma comment( lib, "anwLib1U.lib" )
#pragma comment( lib, "anwImageU.lib" )

#ifdef _DEBUG
#pragma comment( lib, "anwControlUD.lib" )
#else
#pragma comment( lib, "anwControlU.lib" )
#endif

#else
#pragma comment( lib, "anwLib1.lib" )
#pragma comment( lib, "anwImage.lib" )

#ifdef _DEBUG
#pragma comment( lib, "anwControlD.lib" )
#else
#pragma comment( lib, "anwControl.lib" )
#endif

#endif
//#pragma comment( lib, "videomodule.lib" )

/*
//Nono, 2004_0612 =>
#ifdef _DEBUG
#pragma comment( lib, "ListPrintID" )
#else
#pragma comment( lib, "ListPrint" )
#endif
//Nono, 2004_0612 <=
*/
#endif //__ANW_IMPORT_LIB_H


