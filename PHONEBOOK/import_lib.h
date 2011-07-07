#ifndef __ANW_IMPORT_LIB_H
#define __ANW_IMPORT_LIB_H

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

#else //ansi

#pragma comment( lib, "anwLib1.lib" )
#pragma comment( lib, "anwImage.lib" )

#ifdef _DEBUG
#pragma comment( lib, "anwControlD.lib" )
#else
#pragma comment( lib, "anwControl.lib" )
#endif

#endif


#pragma comment( lib, "videomodule.lib" )

#endif //__ANW_IMPORT_LIB_H


