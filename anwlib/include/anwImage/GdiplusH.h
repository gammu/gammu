#ifndef _GDIPLUSH_H_INCLUDED_
#define _GDIPLUSH_H_INCLUDED_

// 
// GDI+ helper file v1.0
// 
// Written by Zoltan Csizmadia (zoltan_csizmadia@yahoo.com)
// 

// GDIPLUS_NO_AUTO_INIT:
// GDI+ won't be initialized at program startup, 
// you have to create GdiPlus::GdiPlusInitialize object to 
// initialize GDI+ ( GDI+ will be uninitialized, when destructor
// is called.
//#define GDIPLUS_NO_AUTO_INIT

// GDIPLUS_USE_GDIPLUS_MEM:
// GdipAlloc and GdipFree is used for memory operations
// In this case _Crt functions cannot be used to detect 
// memory leaks
//#define GDIPLUS_USE_GDIPLUS_MEM

// GDIPLUS_NO_AUTO_NAMESPACE:
// Gdiplus namespace wont' be defined as a used namespace
// In this case you have to use Gdiplus:: prefix
//#define GDIPLUS_NO_AUTO_NAMESPACE

#ifdef _GDIPLUS_H
#error Gdiplus.h is already included. You have to include this file instead.
#endif

// Fix for STL iterator problem
#define iterator _iterator
#define list _list
#define map _map

#define _GDIPLUSBASE_H

namespace Gdiplus
{
    namespace DllExports
    {
        #include "GdiplusMem.h"
    };

    class GdiplusBase
    {
    public:
#ifdef _DEBUG
        static void* __cdecl GdiplusAlloc( size_t nSize, LPCSTR szFileName, int nLine )
        {
#ifdef GDIPLUS_USE_GDIPLUS_MEM
            UNREFERENCED_PARAMETER(szFileName);
            UNREFERENCED_PARAMETER(nLine);
            return DllExports::GdipAlloc(nSize); 
#else
            return ::operator new( nSize, szFileName, nLine );
#endif
        }

        static void GdiplusFree( void* pVoid, LPCSTR szFileName, int nLine )
        {
#ifdef GDIPLUS_USE_GDIPLUS_MEM
            UNREFERENCED_PARAMETER(szFileName);
            UNREFERENCED_PARAMETER(nLine);
            DllExports::GdipFree(pVoid); 
#else
            ::operator delete( pVoid, szFileName, nLine );
#endif
        }
        
        void* (operator new)(size_t nSize) 
        { 
            return GdiplusAlloc( nSize, __FILE__, __LINE__ );
        }
        void* (operator new[])(size_t nSize) 
        { 
            return GdiplusAlloc( nSize, __FILE__, __LINE__ );
        }
        void * (operator new)(size_t nSize, LPCSTR lpszFileName, int nLine) 
        { 
            return GdiplusAlloc( nSize, lpszFileName, nLine );
        }
        void (operator delete)(void* pVoid) 
        { 
            GdiplusFree( pVoid, __FILE__, __LINE__ );
        }
        void (operator delete[])(void* pVoid) 
        { 
            GdiplusFree( pVoid, __FILE__, __LINE__ );
        }
        void operator delete(void* pVoid, LPCSTR lpszFileName, int nLine) 
        { 
            GdiplusFree( pVoid, lpszFileName, nLine);
        }
#else // _DEBUG

        static void* __cdecl GdiplusAlloc( size_t nSize )
        {
#ifdef GDIPLUS_USE_GDIPLUS_MEM
            return DllExports::GdipAlloc(nSize); 
#else
            return ::operator new(nSize);
#endif
        }

        static void GdiplusFree( void* pVoid )
        {
#ifdef GDIPLUS_USE_GDIPLUS_MEM
            DllExports::GdipFree(pVoid); 
#else
            ::operator delete( pVoid );
#endif
        }

        void* (operator new)(size_t nSize) 
        { 
            return GdiplusAlloc( nSize );
        }
        void* (operator new[])(size_t nSize) 
        { 
            return GdiplusAlloc( nSize );
        }
        void (operator delete)(void* pVoid) 
        { 
            GdiplusFree( pVoid );
        }
        void (operator delete[])(void* pVoid) 
        { 
            GdiplusFree( pVoid );
        }
#endif
    };
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <Gdiplus.h> 

#ifdef _DEBUG
#undef new
#endif

#pragma comment (lib, "Gdiplus.lib")

namespace Gdiplus
{
    class GdiPlusInitialize
    {
    public:
        GdiPlusInitialize()
        {
            GdiplusStartupInput Startup;
            GdiplusStartup( &m_Token, &Startup, NULL );
        }

        ~GdiPlusInitialize()
        {
            GdiplusShutdown( m_Token );
        }

    protected:
        ULONG_PTR m_Token;
#ifndef GDIPLUS_NO_AUTO_INIT
        static GdiPlusInitialize m_Initialize;
#endif
    };
#ifndef GDIPLUS_NO_AUTO_INIT
    GdiPlusInitialize GdiPlusInitialize::m_Initialize;
#endif
}

#ifndef GDIPLUS_NO_AUTO_NAMESPACE
using namespace Gdiplus;
#endif

// STL problem
#undef iterator
#undef list
#undef map

#endif
