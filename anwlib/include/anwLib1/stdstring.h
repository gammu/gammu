// =============================================================================
//  FILE:  StdString.h
//  AUTHOR:	Joe O'Leary (with outside help noted in comments)
//  REMARKS:
//		This header file declares the CStdStr template.  This template derives
//		the Standard C++ Library basic_string<> template and add to it the
//		the following conveniences:
//			- The full MFC CString set of functions (including implicit cast)
//			- writing to/reading from COM IStream interfaces
//			- Functional objects for use in STL algorithms
//
//		From this template, we intstantiate two classes:  CStdStringA and
//		CStdStringW.  The name "CStdString" is just a #define of one of these,
//		based upone the _UNICODE macro setting
//
//		This header also declares our own version of the MFC/ATL UNICODE-MBCS
//		conversion macros.  Our version looks exactly like the Microsoft's to
//		facilitate portability.
//
//	NOTE:
//		If you you use this in an MFC or ATL build, you should include either
//		afx.h or atlbase.h first, as appropriate.
//
//	PEOPLE WHO HAVE CONTRIBUTED TO THIS CLASS:
//
//		Several people have helped me iron out problems and othewise improve
//		this class.  OK, this is a long list but in my own defense, this code
//		has undergone two major rewrites.  Many of the improvements became
//		necessary after I rewrote the code as a template.  Others helped me
//		improve the CString facade.
//
//		Anyway, these people are (in chronological order):
//
//			- Pete the Plumber (???)
//			- Julian Selman
//			- Chris (of Melbsys)
//			- Dave Plummer
//			- John C Sipos
//			- Chris Sells
//			- Nigel Nunn
//			- Fan Xia
//			- Matthew Williams
//			- Carl Engman
//			- Mark Zeren
//			- Craig Watson
//			- Rich Zuris
//			- Karim Ratib
//			- Chris Conti
//			- Baptiste Lepilleur
//			- Greg Pickles
//			- Jim Cline
//			- Jeff Kohn
//
//	REVISION HISTORY
//
//    2000-FEB-01 - You can now use CStdString on the Mac with CodeWarrior!
//					Thanks to Todd Heckel for helping out with this.
//	  2000-JAN-23 - Thanks to Jim Cline for pointing out how I could make the
//					Trim() function more efficient.
//				  - Thanks to Jeff Kohn for prompting me to find and fix a typo
//					in one of the addition operators that takes _bstr_t.
//				  - Got rid of the .CPP file -  you only need StdString.h now!
//
//	  1999-DEC-22 - Thanks to Greg Pickles for helping me identify a problem
//					with my implementation of CStdString::FormatV in which
//					resulting string might not be properly NULL terminated.
//
//	  1999-DEC-06 - Chris Conti pointed yet another basic_string<> assignment
//					bug that MS has not fixed.  CStdString did nothing to fix
//					it either but it does now!  The bug was: create a string
//					longer than 31 characters, get a pointer to it (via c_str())
//					and then assign that pointer to the original string object.
//					The resulting string would be empty.  Not with CStdString!
//
//	  1999-OCT-06 - BufferSet was erasing the string even when it was merely
//					supposed to shrink it.  Fixed.  Thanks to Chris Conti.
//				  - Some of the Q172398 fixes were not checking for assignment-
//					to-self.  Fixed.  Thanks to Baptiste Lepilleur.
//
//	  1999-AUG-20 - Improved Load() function to be more efficient by using 
//					SizeOfResource().  Thanks to Rich Zuris for this.
//				  - Corrected resource ID constructor, again thanks to Rich.
//				  - Fixed a bug that occurred with UNICODE characters above
//					the first 255 ANSI ones.  Thanks to Craig Watson. 
//				  - Added missing overloads of TrimLeft() and TrimRight().
//					Thanks to Karim Ratib for pointing them out
//
//	  1999-JUL-21 - Made all calls to Buffer() with no args check length first.
//
//	  1999-JUL-10 - Improved MFC/ATL independence of conversion macros
//				  - Added SS_NO_REFCOUNT macro to allow you to disable any
//					reference-counting your basic_string<> impl. may do.
//				  - Improved ReleaseBuffer() to be as forgiving as CString.
//					Thanks for Fan Xia for helping me find this and to
//					Matthew Williams for pointing it out directly.
//
//	  1999-JUL-06 - Thanks to Nigel Nunn for catching a very sneaky bug in
//					ToLower/ToUpper.  They should call Buffer() instead of
//					data() in order to ensure the changed string buffer is not
//					reference-counted (in those implementations that refcount).
//
//	  1999-JUL-01 - Added a true CString facade.  Now you can use CStdString as
//					a drop-in replacement for CString.  If you find this useful,
//					you can thank Chris Sells for finally convincing me to give
//					in and implement it.
//				  - Changed operators << and >> (for MFC CArchive) to serialize
//					EXACTLY as CString's do.  So now you can send a CString out
//					to a CArchive and later read it in as a CStdString.   I have
//					no idea why you would want to do this but you can. 
//
//	  1999-JUN-21 - Changed the CStdString class into the CStdStr template.
//				  - Fixed FormatV() to correctly decrement the loop counter.
//					This was harmless bug but a bug nevertheless.  Thanks to
//					Chris (of Melbsys) for pointing it out
//				  - Changed Format() to try a normal stack-based array before
//					using to _alloca().
//				  - Updated the text conversion macros to properly use code
//					pages and to fit in better in MFC/ATL builds.  In other
//					words, I copied Microsoft's conversion stuff again. 
//				  - Added equivalents of CString::GetBuffer, GetBufferSetLength
//				  - new sscpy() replacement of CStdString::CopyString()
//				  - a Trim() function that combines TrimRight() and TrimLeft().
//
//	  1999-MAR-13 - Corrected the "NotSpace" functional object to use _istpace()
//					instead of _isspace()   Thanks to Dave Plummer for this.
//
//	  1999-FEB-26 - Removed errant line (left over from testing) that #defined
//					_MFC_VER.  Thanks to John C Sipos for noticing this.
//
//	  1999-FEB-03 - Fixed a bug in a rarely-used overload of operator+() that
//					caused infinite recursion and stack overflow
//				  - Added member functions to simplify the process of
//					persisting CStdStrings to/from DCOM IStream interfaces 
//				  - Added functional objects (e.g. StdStringLessNoCase) that
//					allow CStdStrings to be used as keys STL map objects with
//					case-insensitive comparison 
//				  - Added array indexing operators (i.e. operator[]).  I
//					originally assumed that these were unnecessary and would be
//					inherited from basic_string.  However, without them, Visual
//					C++ complains about ambiguous overloads when you try to use
//					them.  Thanks to Julian Selman to pointing this out. 
//
//	  1998-FEB-?? - Added overloads of assign() function to completely account
//					for Q172398 bug.  Thanks to "Pete the Plumber" for this
//
//	  1998-FEB-?? - Initial submission
//
// COPYRIGHT:
//		1999 Joseph M. O'Leary.  This code is free.  Use it anywhere you want.
//		Rewrite it, restructure it, whatever.  Please don't blame me if it makes
//		your $30 billion dollar satellite explode in orbit.  If you redistribute
//		it in any form, I'd appreciate it if you would leave this notice here.
//
//		If you find any bugs, please let me know:
//
//				jmoleary@earthlink.net
//				http://home.earthlink.net/~jmoleary
// =============================================================================

// Avoid multiple inclusion the VC++ way,
// Turn off browser references
// Turn off unavoidable compiler warnings

#if defined(_MSC_VER) && (_MSC_VER > 1100)
	#pragma once
	#pragma component(browser, off, references, "CStdString")
	#pragma warning (disable : 4290) // C++ Exception Specification ignored
	#pragma warning (disable : 4127) // Conditional expression is constant
	#pragma warning (disable : 4097) // typedef name used as synonym for class name
#endif

#ifndef _STDSTRING_H_
#define _STDSTRING_H_


// MACRO: SS_NO_REFCOUNT:
//		turns off reference counting at the assignment level
//		I define this by default.  comment it out if you don't want it.

#define SS_NO_REFCOUNT	

// In non-Visual C++ and/or non-Win32 builds, we can't use some cool stuff.

#if !defined(_MSC_VER) || !defined(_WIN32)
	#define SS_ANSI
#endif

// Avoid legacy code screw up: if _UNICODE is defined, UNICODE must be as well

#if defined (_UNICODE) && !defined (UNICODE)
	#define UNICODE
#endif
#if defined (UNICODE) && !defined (_UNICODE)
	#define _UNICODE
#endif

// If this class is not being built as a part of the W32 library, then we have
// not yet included W32Base.h so we've got to include and define all the
// necessary stuff here.

#if !defined(_W32BASE_H_)

	template<class Type>
	inline const Type& SSMIN(const Type& arg1, const Type& arg2)
	{
		return arg2 < arg1 ? arg2 : arg1;
	}

	// If they want us to use only standard C++ stuff (no Win32 stuff)

	#ifdef SS_ANSI

		// On non-Win32 platforms, there is no TCHAR.H so define what we need

		#ifndef _WIN32


			typedef const char*		PCSTR;
			typedef char*			PSTR;
			typedef const wchar_t*	PCWSTR;
			typedef wchar_t*		PWSTR;
			#ifdef UNICODE
				typedef wchar_t		TCHAR;
			#else
				typedef char		TCHAR;
			#endif
			typedef wchar_t			OLECHAR;

		#else

			#include <TCHAR.H>
			#include <WTYPES.H>
			#ifndef STRICT
				#define STRICT
			#endif

		#endif	// #ifndef _WIN32


		// Make sure ASSERT and verify are defined in an ANSI fashion

		#ifndef ASSERT
			#include <assert.h>
			#define ASSERT(f) assert((f))
		#endif
		#ifndef VERIFY
			#ifdef _DEBUG
				#define VERIFY(x) ASSERT((x))
			#else
				#define VERIFY(x) x
			#endif
		#endif

	#else // #ifdef SS_ANSI

		#include <TCHAR.H>
		#include <WTYPES.H>
		#ifndef STRICT
			#define STRICT
		#endif

//		#include <windows.h>

		// Make sure ASSERT and verify are defined

		#ifndef ASSERT
			#include <crtdbg.h>
			#define ASSERT(f) _ASSERTE((f))
		#endif
		#ifndef VERIFY
			#ifdef _DEBUG
				#define VERIFY(x) ASSERT((x))
			#else
				#define VERIFY(x) x
			#endif
		#endif

	#endif // #ifdef SS_ANSI

#else 

	#define NSP std
	#define SSMIN W32MIN

#endif // #ifndef _W32BASE_H_

// Standard headers needed

#include <string>			// basic_string
#include <algorithm>
#include <functional>		// for StdStringLessNoCase, et al
#include <locale>			// for various facets

// MACRO: NSP
// Defines the name of the namespace the standard library resides in.
// Some implementations place the standard library in other namespaces or in
// no namespace at all.  You can just change this definition to use whatever
// you want
// Note:  This technique results in global scope being explicitly used if you
// #define NSP nothing.  (e.g. '::wstring' instead of just 'wstring').  I
// Suppose this could interfere with Koenig Lookup, but I'm taking that chance.

#ifndef NSP
	#define NSP				std
#endif

// If this is a recent enough version of VC include comdef.h, so we can write
// member functions to deal with COM types & compiler support classes e.g. _bstr_t

#if defined (_MSC_VER) && (_MSC_VER >= 1100)
	#include <comdef.h>
	#define SS_INC_COMDEF		// signal that we #included MS comdef.h file
	#define STDSTRING_INC_COMDEF
#endif

#ifndef TRACE
	#define TRACE_DEFINED_HERE
	#define TRACE
#endif

// Microsoft defines PCSTR, PCWSTR, etc, but no PCTSTR.  I hate to use the
// versions with the "L" in front of them because that's a leftover from Win 16
// days, even though it evaluates to the same thing.  Therefore, Define a PCSTR
// as an LPCTSTR.

#if !defined(PCTSTR) && !defined(PCTSTR_DEFINED)
	typedef const TCHAR*			PCTSTR;
	#define PCTSTR_DEFINED
#endif

#if !defined(PCOLESTR) && !defined(PCOLESTR_DEFINED)
	typedef const OLECHAR*			PCOLESTR;
	#define PCOLESTR_DEFINED
#endif

#if !defined(POLESTR) && !defined(POLESTR_DEFINED)
	typedef OLECHAR*				POLESTR;
	#define POLESTR_DEFINED
#endif

#if !defined(PCUSTR) && !defined(PCUSTR_DEFINED)
	typedef const unsigned char*	PCUSTR;
	typedef unsigned char*			PUSTR;
	#define PCUSTR_DEFINED
#endif

// SS_USE_FACET macro and why we need it:
//
// Since I'm a good little Standard C++ programmer, I use locales.  Thus, I
// need to make use of the use_facet<> template function here.   Unfortunately,
// this need is complicated by the fact the MS' implementation of the Standard
// C++ Library has a non-standard version of use_facet that takes more
// arguments than the standard dictates.  Since I'm trying to write CStdString
// to work with any version of the Standard library, this presents a problem.
//
// The upshot of this is that I can't do 'use_facet' directly.  The MS' docs
// tell me that I have to use a macro, _USE() instead.  Since _USE obviously
// won't be available in other implementations, this means that I have to write
// my OWN macro -- SS_USE_FACET -- that evaluates either to _USE or to the
// standard, use_facet.
//
// If you are having trouble with the SS_USE_FACET macro, in your implementation
// of the Standard C++ Library, you can define your own version of SS_USE_FACET.

#ifndef SS_USE_FACET
	#ifdef _MSC_VER 
		#define SS_USE_FACET(loc, fac) NSP::_USE(loc,fac)
	#else
		#define SS_USE_FACET(loc, fac) NSP::use_facet<fac>(loc)
	#endif
#endif

// =============================================================================
// UNICODE/MBCS conversion macros.  Made to work just like the MFC/ATL ones.
// =============================================================================

// First define the conversion helper functions.  We define these regardless of
// any preprocessor macro settings since their names won't collide. 

#ifdef SS_ANSI // Are we doing things the standard, non-Win32 way?...

	typedef NSP::codecvt<wchar_t, char, mbstate_t> SSCodeCvt;

	// Not sure if we need all these headers.   I believe ANSI says we do.

	#include <stdio.h>
	#include <stdarg.h>
	#include <wchar.h>
	#ifndef va_start
		#include <varargs.h>
	#endif

	inline PWSTR StdCodeCvt(PWSTR pW, PCSTR pA, int nChars)
	{
		PSTR pBadA				= NULL;
		PWSTR pBadW				= NULL;
		SSCodeCvt::result res	= SSCodeCvt::ok;
		const SSCodeCvt& conv	= SS_USE_FACET(NSP::locale(), SSCodeCvt);
		res						= conv.in(res,
										  pA, pA + nChars, pBadA,
										  pW, pW + nChars, pBadW);
		ASSERT(SSCodeCvt::ok == res);
		return pW;
	}

	inline PSTR StdCodeCvt(PSTR pA, PCWSTR pW, int nChars)
	{
		PSTR pBadA				= NULL;
		PWSTR pBadW				= NULL;
		const SSCodeCvt& conv	= SS_USE_FACET(NSP::locale(), SSCodeCvt);
		SSCodeCvt::result res	= SSCodeCvt::ok;
		res						= conv.out(res,
										   pW, pW + nChars, pBadW,
										   pA, pA + nChars, pBadA);
		ASSERT(SSCodeCvt::ok == res);
		return pA;
	}

#else   // ...or are we doing things assuming win32 and Visual C++?

	#include <malloc.h>	// needed for _alloca

	inline PWSTR StdCodeCvt(PWSTR pW, PCSTR pA, int nChars, UINT acp=CP_ACP)
	{
		ASSERT(pA != NULL);
		ASSERT(pW != NULL);
		pW[0] = '\0';
		MultiByteToWideChar(acp, 0, pA, -1, pW, nChars);
		return pW;
	}

	inline PSTR StdCodeCvt(PSTR pA, PCWSTR pW, int nChars, UINT acp=CP_ACP)
	{
		ASSERT(pA != NULL);
		ASSERT(pW != NULL);
		pA[0] = '\0';
		WideCharToMultiByte(acp, 0, pW, -1, pA, nChars, NULL, NULL);
		return pA;
	}

	// Define our conversion macros to look exactly like Microsoft's to
	// facilitate using this stuff both with and without MFC/ATL

	#ifdef _CONVERSION_USES_THREAD_LOCALE
		#ifndef _DEBUG
			#define SSCVT int _cvt; _cvt; UINT _acp=GetACP(); \
				_acp; PCWSTR _pw; _pw; PCSTR _pa; _pa
		#else
			#define SSCVT int _cvt = 0; _cvt; UINT _acp=GetACP();\
				 _acp; PCWSTR _pw=NULL; _pw; PCSTR _pa=NULL; _pa
		#endif
	#else
		#ifndef _DEBUG
			#define SSCVT int _cvt; _cvt; UINT _acp=CP_ACP; _acp;\
				 PCWSTR _pw; _pw; PCSTR _pa; _pa
		#else
			#define SSCVT int _cvt = 0; _cvt; UINT _acp=CP_ACP; \
				_acp; PCWSTR _pw=NULL; _pw; PCSTR _pa=NULL; _pa
		#endif
	#endif

	#ifdef _CONVERSION_USES_THREAD_LOCALE
		#define SSA2W(pa) (\
			((_pa = pa) == NULL) ? NULL : (\
				_cvt = (strlen(_pa)+1),\
				StdCodeCvt((PWSTR) _alloca(_cvt*2), _pa, _cvt, _acp)))
		#define SSW2A(pw) (\
			((_pw = pw) == NULL) ? NULL : (\
				_cvt = (wcslen(_pw)+1)*2,\
				StdW2AHelper((LPSTR) _alloca(_cvt), _pw, _cvt, _acp)))
	#else
		#define SSA2W(pa) (\
			((_pa = pa) == NULL) ? NULL : (\
				_cvt = (strlen(_pa)+1),\
				StdCodeCvt((PWSTR) _alloca(_cvt*2), _pa, _cvt)))
		#define SSW2A(pw) (\
			((_pw = pw) == NULL) ? NULL : (\
				_cvt = (wcslen(_pw)+1)*2,\
				StdCodeCvt((LPSTR) _alloca(_cvt), _pw, _cvt)))
	#endif

	#define SSA2CW(pa) ((PCWSTR)SSA2W((pa)))
	#define SSW2CA(pw) ((PCSTR)SSW2A((pw)))

	#ifdef UNICODE
		#define SST2A	SSW2A
		#define SSA2T	SSA2W
		#define SST2CA	SSW2CA
		#define SSA2CT	SSA2CW
		inline PWSTR	SST2W(PTSTR p)			{ return p; }
		inline PTSTR	SSW2T(PWSTR p)			{ return p; }
		inline PCWSTR	SST2CW(PCTSTR p)		{ return p; }
		inline PCTSTR	SSW2CT(PCWSTR p)		{ return p; }
	#else
		#define SST2W	SSA2W
		#define SSW2T	SSW2A
		#define SST2CW	SSA2CW
		#define SSW2CT	SSW2CA
		inline PSTR		SST2A(PTSTR p)			{ return p; }
		inline PTSTR	SSA2T(PSTR p)			{ return p; }
		inline PCSTR	SST2CA(PCTSTR p)		{ return p; }
		inline PCTSTR	SSA2CT(PCSTR p)			{ return p; }
	#endif // #ifdef UNICODE

	#if defined(UNICODE)
	// in these cases the default (TCHAR) is the same as OLECHAR
		inline PCOLESTR	SST2COLE(PCTSTR p)		{ return p; }
		inline PCTSTR	SSOLE2CT(PCOLESTR p)	{ return p; }
		inline POLESTR	SST2OLE(PTSTR p)		{ return p; }
		inline PTSTR	SSOLE2T(POLESTR p)		{ return p; }
	#elif defined(OLE2ANSI)
	// in these cases the default (TCHAR) is the same as OLECHAR
		inline PCOLESTR	SST2COLE(PCTSTR p)		{ return p; }
		inline PCTSTR	SSOLE2CT(PCOLESTR p)	{ return p; }
		inline POLESTR	SST2OLE(PTSTR p)		{ return p; }
		inline PTSTR	SSOLE2T(POLESTR p)		{ return p; }
	#else
		//CharNextW doesn't work on Win95 so we use this
		#define SST2COLE(pa)	SSA2CW((pa))
		#define SST2OLE(pa)		SSA2W((pa))
		#define SSOLE2CT(po)	SSW2CA((po))
		#define SSOLE2T(po)		SSW2A((po))
	#endif

	#ifdef OLE2ANSI
		#define SSW2OLE		SSW2A
		#define SSOLE2W		SSA2W
		#define SSW2COLE	SSW2CA
		#define SSOLE2CW	SSA2CW
		inline POLESTR		SSA2OLE(PSTR p)		{ return p; }
		inline PSTR			SSOLE2A(POLESTR p)	{ return p; }
		inline PCOLESTR		SSA2COLE(PCSTR p)	{ return p; }
		inline PCSTR		SSOLE2CA(PCOLESTR p){ return p; }
	#else
		#define SSA2OLE		SSA2W
		#define SSOLE2A		SSW2A
		#define SSA2COLE	SSA2CW
		#define SSOLE2CA	SSW2CA
		inline POLESTR		SSW2OLE(PWSTR p)	{ return p; }
		inline PWSTR		SSOLE2W(POLESTR p)	{ return p; }
		inline PCOLESTR		SSW2COLE(PCWSTR p)	{ return p; }
		inline PCWSTR		SSOLE2CW(PCOLESTR p){ return p; }
	#endif

	// Above we've defined macros that look like MS' but all have
	// an 'SS' prefix.  Now we need the real macros.  We'll either
	// get them from the macros above or from MFC/ATL.  If
	// SS_NO_CONVERSION is #defined, we'll forgo them

	#ifndef SS_NO_CONVERSION

		#if defined (USES_CONVERSION)

			#define _NO_STDCONVERSION	// just to be consistent

		#else

			#ifdef _MFC_VER

				#include <afxconv.h>
				#define _NO_STDCONVERSION // just to be consistent

			#else

				#define USES_CONVERSION SSCVT
				#define A2CW			SSA2CW
				#define W2CA			SSW2CA
				#define T2A				SST2A
				#define A2T				SSA2T
				#define T2W				SST2W
				#define W2T				SSW2T
				#define T2CA			SST2CA
				#define A2CT			SSA2CT
				#define T2CW			SST2CW
				#define W2CT			SSW2CT
				#define ocslen			sslen
				#define ocscpy			sscpy
				#define T2COLE			SST2COLE
				#define OLE2CT			SSOLE2CT
				#define T2OLE			SST2COLE
				#define OLE2T			SSOLE2CT
				#define A2OLE			SSA2OLE
				#define OLE2A			SSOLE2A
				#define W2OLE			SSW2OLE
				#define OLE2W			SSOLE2W
				#define A2COLE			SSA2COLE
				#define OLE2CA			SSOLE2CA
				#define W2COLE			SSW2COLE
				#define OLE2CW			SSOLE2CW
		
			#endif // #ifdef _MFC_VER
		#endif // #ifndef USES_CONVERSION
	#endif // #ifndef SS_NO_CONVERSION

	// Define ostring - generic name for NSP::basic_string<OLECHAR>

	#if !defined(ostring) && !defined(OSTRING_DEFINED)
		typedef NSP::basic_string<OLECHAR> ostring;
		#define OSTRING_DEFINED
	#endif

#endif // #ifndef SS_ANSI

// Define tstring -- generic name for NSP::basic_string<TCHAR>

#if !defined(tstring) && !defined(TSTRING_DEFINED)
	typedef NSP::basic_string<TCHAR> tstring;
	#define TSTRING_DEFINED
#endif

// a very shorthand way of applying the fix for KB problem Q172398
// (basic_string assignment bug)

#if defined ( _MSC_VER ) && ( _MSC_VER < 1200 )
	#define Q172398(x) (x).erase()
#else
	#define Q172398(x)
#endif

// =============================================================================
// INLINE FUNCTIONS ON WHICH CSTDSTRING RELIES
//
// Usually for generic text mapping, we rely on preprocessor macro definitions
// to map to string functions.  However the CStdStr<> template cannot use
// macro-based generic text mappings because its character types do not get
// resolved until template processing which comes AFTER macro processing.  In
// other words, UNICODE is of little help to us in the CStdStr template
//
// Therefore, to keep the CStdStr declaration simple, we have these inline
// functions.  The template calls them often.  Since they are inline (and NOT
// exported when this is built as a DLL), they will probably be resolved away
// to nothing. 
//
// Without these functions, the CStdStr<> template would probably have to broken
// out into two, almost identical classes.  Either that or it would be a huge,
// convoluted mess, with tons of "if" statements all over the place checking the
// size of template parameter CT.
// 
// In several cases, you will see two versions of each function, with one
// commented out.  The second version is the more portable, standard way of
// doing things, while the first version is non-standard, but significantly
// faster.  Uncomment whichever one suits your needs
// =============================================================================

// If they defined SS_NO_REFCOUNT, then we must convert all assignments

#ifdef SS_NO_REFCOUNT
	#define SSREF(x) (x).c_str()
#else
	#define SSREF(x) (x)
#endif

// -----------------------------------------------------------------------------
// strlen/wcslen wrappers
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
	template<typename CT> inline long sslen(const CT* pT)
	{
		return NULL == pT ? 0 : NSP::char_traits<CT>::length(pT);
	}
#else
	inline size_t sslen(PCSTR pA)
	{
		return NULL == pA ? 0 : strlen(pA);
	}
	inline size_t sslen(PCWSTR pW)
	{
		return NULL == pW ? 0 : wcslen(pW);
	}
#endif


// -----------------------------------------------------------------------------
// assignment functions -- assign "sSrc" to "sDst"
// -----------------------------------------------------------------------------
inline void	ssasn(NSP::string& sDst, const NSP::string& sSrc)
{
	if ( sDst.c_str() != sSrc.c_str() )
	{
		sDst.erase();
		sDst.assign(SSREF(sSrc));
	}
}
inline void	ssasn(NSP::string& sDst, PCSTR pA)
{
	// Watch out for NULLs, as always.

	if ( NULL == pA )
	{
		sDst.erase();
	}

	// If pA actually points to part of sDst, we must NOT erase(), but
	// rather take a substring

	else if ( pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.size() )
	{
		sDst	= sDst.substr(pA-sDst.c_str());
	}

	// Otherwise (most cases) apply the assignment bug fix, if applicable
	// and do the assignment

	else
	{
		Q172398(sDst);
		sDst.assign(pA);
	}
}
inline void	ssasn(NSP::string& sDst, const NSP::wstring& sSrc)
{
#ifdef SS_ANSI
	int nLen	= sSrc.size();
	sDst.resize(0);
	sDst.resize(nLen);
	StdCodeCvt(const_cast<NSP::string::pointer>(sDst.data()),
				 sSrc.c_str(),
				 nLen);
#else
	SSCVT;
	sDst.assign(SSW2CA(sSrc.c_str()));
#endif
}
inline void	ssasn(NSP::string& sDst, PCWSTR pW)
{
#ifdef SS_ANSI
	int nLen	= sslen(pW);
	sDst.resize(0);
	sDst.resize(nLen);
	StdCodeCvt(const_cast<NSP::string::pointer>(sDst.data()), pW, nLen);
#else
	SSCVT;
	sDst.assign(pW ? SSW2CA(pW) : "");
#endif
}
inline void ssasn(NSP::string& sDst, const int nNull)
{
	ASSERT(nNull==NULL);
	sDst.assign("");
}	
inline void	ssasn(NSP::wstring& sDst, const NSP::wstring& sSrc)
{
	if ( sDst.c_str() != sSrc.c_str() )
	{
		sDst.erase();
		sDst.assign(SSREF(sSrc));
	}
}
inline void	ssasn(NSP::wstring& sDst, PCWSTR pW)
{
	// Watch out for NULLs, as always.

	if ( NULL == pW )
	{
		sDst.erase();
	}

	// If pW actually points to part of sDst, we must NOT erase(), but
	// rather take a substring

	else if ( pW >= sDst.c_str() && pW <= sDst.c_str() + sDst.size() )
	{
		sDst	= sDst.substr(pW-sDst.c_str());
	}

	// Otherwise (most cases) apply the assignment bug fix, if applicable
	// and do the assignment

	else
	{
		Q172398(sDst);
		sDst.assign(pW);
	}
}
inline void	ssasn(NSP::wstring& sDst, const NSP::string& sSrc)
{
#ifdef SS_ANSI
	int nLen	= sSrc.size();
	sDst.resize(0);
	sDst.resize(nLen);
	StdCodeCvt(const_cast<NSP::wstring::pointer>(sDst.data()),
				 sSrc.c_str(),
				 nLen);
#else
	SSCVT;
	sDst.assign(SSA2CW(sSrc.c_str()));
#endif
}
inline void	ssasn(NSP::wstring& sDst, PCSTR pA)
{
#ifdef SS_ANSI
	int nLen	= sslen(pA);
	sDst.resize(0);
	sDst.resize(nLen);
	StdCodeCvt(const_cast<NSP::wstring::pointer>(sDst.data()), pA, nLen);
#else
	SSCVT;
	sDst.assign(pA ? SSA2CW(pA) : L"");
#endif
}
inline void ssasn(NSP::wstring& sDst, const int nNull)
{
	ASSERT(nNull==NULL);
	sDst.assign(L"");
}


// -----------------------------------------------------------------------------
// string object concatenation -- add second argument to first
// -----------------------------------------------------------------------------
inline void	ssadd(NSP::string& sDst, const NSP::wstring& sSrc)
{
#ifdef SS_ANSI
	int nLen	= sSrc.size();
	sDst.resize(sDst.size() + nLen);
	StdCodeCvt(const_cast<NSP::string::pointer>(sDst.data()+nLen),
			   sSrc.c_str(),
			   nLen);
#else
	SSCVT; 
	sDst.append(SSW2CA(sSrc.c_str())); 
#endif
}
inline void	ssadd(NSP::string& sDst, const NSP::string& sSrc)
{ 
	sDst.append(sSrc.c_str());
}
inline void	ssadd(NSP::string& sDst, PCWSTR pW)
{
#ifdef SS_ANSI
	int nLen	= sslen(pW);
	sDst.resize(sDst.size() + nLen);
	StdCodeCvt(const_cast<NSP::string::pointer>(sDst.data()+nLen), pW, nLen);
#else
	SSCVT;
	if ( NULL != pW )
		sDst.append(SSW2CA(pW)); 
#endif
}
inline void	ssadd(NSP::string& sDst, PCSTR pA)
{
	if ( pA )
		sDst.append(pA); 
}
inline void	ssadd(NSP::wstring& sDst, const NSP::wstring& sSrc)
{
	sDst.append(sSrc.c_str());
}
inline void	ssadd(NSP::wstring& sDst, const NSP::string& sSrc)
{
#ifdef SS_ANSI
	int nLen	= sSrc.size();
	sDst.resize(sDst.size() + nLen);
	StdCodeCvt(const_cast<NSP::wstring::pointer>(sDst.data()+nLen),
				 sSrc.c_str(),
				 nLen);
#else
	SSCVT;
	sDst.append(SSA2CW(sSrc.c_str()));
#endif
}
inline void	ssadd(NSP::wstring& sDst, PCSTR pA)
{
#ifdef SS_ANSI
	int nLen	= sslen(pA);
	sDst.resize(sDst.size() + nLen);
	StdCodeCvt(const_cast<NSP::wstring::pointer>(sDst.data()+nLen), pA, nLen);
#else
	SSCVT;
	if ( NULL != pA )
		sDst.append(SSA2CW(pA));
#endif
}
inline void	ssadd(NSP::wstring& sDst, PCWSTR pW)
{
	if ( pW )
		sDst.append(pW);
}


// -----------------------------------------------------------------------------
// comparison (case insensitive )
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
	template<typename CT>
	inline int ssicmp(const CT* pA1, const CT* pA2)
	{
		NSP::locale loc;
		const NSP::ctype<CT>& ct = SS_USE_FACET(loc, NSP::ctype<CT>);
		CT f;
		CT l;

            do 
			{
				f = ct.tolower(*(pA1++));
				l = ct.tolower(*(pA2++));
            } while ( (f) && (f == l) );

        return (int)(f - l);
	}
#else
	#ifdef _MBCS
		inline long sscmp(PCSTR pA1, PCSTR pA2)
		{
			return _mbscmp((PCUSTR)pA1, (PCUSTR)pA2);
		}
		inline long ssicmp(PCSTR pA1, PCSTR pA2)
		{
			return _mbsicmp((PCUSTR)pA1, (PCUSTR)pA2);
		}
	#else
		inline long sscmp(PCSTR pA1, PCSTR pA2)
		{
			return strcmp(pA1, pA2);
		}
		inline long ssicmp(PCSTR pA1, PCSTR pA2)
		{
			return _stricmp(pA1, pA2);
		}
	#endif
	inline long sscmp(PCWSTR pW1, PCWSTR pW2)
	{
		return wcscmp(pW1, pW2);
	}
	inline long ssicmp(PCWSTR pW1, PCWSTR pW2)
	{
		return _wcsicmp(pW1, pW2);
	}
#endif

// -----------------------------------------------------------------------------
// isspace() equivalents
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
	template<typename CT>
	inline bool ssisp(CT t)
	{
		return NSP::isspace(t, NSP::locale());
	}
#else
	inline bool	ssisp(char t)
	{
		return isspace(t) != 0;
	}
	inline bool ssisp(wchar_t t)
	{
		return iswspace(t) != 0;
	}
#endif

// -----------------------------------------------------------------------------
// Uppercase/Lowercase conversion functions
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
	template<typename CT>
	inline void sslwr(CT* pT, size_t nLen)
	{
		SS_USE_FACET(NSP::locale(), NSP::ctype<CT>).tolower(pT, pT+nLen);
	}
	template<typename CT>
	inline void ssupr(CT* pT)
	{
		SS_USE_FACET(NSP::locale(), NSP::ctype<CT>).toupper(pT, pT+nLen);
	}
#else  // #else we must be on Win32
	#ifdef _MBCS
		inline void	ssupr(PSTR pA, size_t /*nLen*/)
		{
			_mbsupr((PUSTR)pA);
		}
		inline void	sslwr(PSTR pA, size_t /*nLen*/)
		{
			_mbslwr((PUSTR)pA);
		}
	#else
		inline void	ssupr(PSTR pA, size_t /*nLen*/)
		{
			_strupr(pA); 
		}
		inline void	sslwr(PSTR pA, size_t /*nLen*/)
		{
			_strlwr(pA);
		}
	#endif
	inline void	ssupr(PWSTR pW, size_t /*nLen*/)	
	{
		_wcsupr(pW);
	}
	inline void	sslwr(PWSTR pW, size_t /*nLen*/)	
	{
		_wcslwr(pW);
	}
#endif // #ifdef SS_ANSI

// -----------------------------------------------------------------------------
//  vsprintf/vswprintf or _vsnprintf/_vsnwprintf equivalents.  In standard
//  builds we can't use _vsnprintf/_vsnwsprintf because they're MS extensions.
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
	inline int ssvsprintf(PSTR pA, size_t /*nCount*/, PCSTR pFmtA, va_list vl)
	{
		return vsprintf(pA, pFmtA, vl);
	}
	inline int ssvsprintf(PWSTR pW, size_t nCount, PCWSTR pFmtW, va_list vl)
	{
	#ifdef __MWERKS__
		return vswprintf(pW, nCount, pFmtW, vl);
	#else
		nCount;
		return vswprintf(pW, pFmtW, vl);
	#endif
	}
	inline int ssvsprintf(PWSTR pW, PCWSTR pFmtW, va_list vl)
	{
		return vswprintf(pW, pFmtW, vl);
	}
#else
	inline int	ssnprintf(PSTR pA, size_t nCount, PCSTR pFmtA, va_list vl)
	{ 
		return _vsnprintf(pA, nCount, pFmtA, vl);
	}
	inline int	ssnprintf(PWSTR pW, size_t nCount, PCWSTR pFmtW, va_list vl)
	{
		return _vsnwprintf(pW, nCount, pFmtW, vl);
	}
#endif


// -----------------------------------------------------------------------------
// Type safe, overloaded ::LoadString wrappers
// There is no equivalent of these in non-Win32-specific builds.
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
#else
	inline int ssload(HMODULE hInst, UINT uId, PSTR pBuf, int nMax)
	{
		return ::LoadStringA(hInst, uId, pBuf, nMax);
	}
	inline int ssload(HMODULE hInst, UINT uId, PWSTR pBuf, int nMax)
	{
		return ::LoadStringW(hInst, uId, pBuf, nMax);
	}
#endif

// -----------------------------------------------------------------------------
// Collation wrappers
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
	template <typename CT>
	inline int sscoll(const CT* sz1, int nLen1, const CT* sz2, int nLen2)
	{
		NSP::collate<CT>& coll	= SS_USE_FACET(NSP::locale(), NSP::collate<CT>);
		return coll.compare(sz1, sz1+nLen1, sz2, sz2+nLen2);
	}
	template <typename CT>
	inline int ssicoll(const CT* sz1, int nLen1, const CT* sz2, int nLen2)
	{
		NSP::collate<CT>& coll	= SS_USE_FACET(NSP::locale(), NSP::collate<CT>);
		coll.string_type s1(sz1);
		coll.string_type s2(sz2);
		sslwr(s1.Buffer(), nLen1);
		sslwr(s2.Buffer(), nLen2);
		return coll.compare(s1.data(), s1.data()+nLen1,
							s2.data(), sz2.data()+nLen2);
	}
#else
	#ifdef _MBCS
		inline int sscoll(PCSTR sz1, int /*nLen1*/, PCSTR sz2, int /*nLen2*/)
		{
			return _mbscoll((PCUSTR)sz1, (PCUSTR)sz2);
		}
		inline int ssicoll(PCSTR sz1, int /*nLen1*/, PCSTR sz2, int /*nLen2*/)
		{
			return _mbsicoll((PCUSTR)sz1, (PCUSTR)sz2);
		}
	#else
		inline int sscoll(PCSTR sz1, int /*nLen1*/, PCSTR sz2, int /*nLen2*/)
		{
			return strcoll(sz1, sz2);
		}
		inline int ssicoll(PCSTR sz1, int /*nLen1*/, PCSTR sz2, int /*nLen2*/)
		{
			return _stricoll(sz1, sz2);
		}
	#endif
	inline int sscoll(PCWSTR sz1, int /*nLen1*/, PCWSTR sz2, int /*nLen2*/)
	{
		return wcscoll(sz1, sz2);
	}
	inline int ssicoll(PCWSTR sz1, int /*nLen1*/, PCWSTR sz2, int /*nLen2*/)
	{
		return _wcsicoll(sz1, sz2);
	}
#endif

// -----------------------------------------------------------------------------
// FormatMessage equivalents.  Needed because I added a CString facade
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
#else
	inline DWORD ssfmtmsg(DWORD dwFlags, LPCVOID pSrc, DWORD dwMsgId,
						  DWORD dwLangId, PSTR pBuf, DWORD nSize,
						  va_list* vlArgs)
	{ 
		return FormatMessageA(dwFlags, pSrc, dwMsgId, dwLangId,
							  pBuf, nSize,vlArgs);
	}
	inline DWORD ssfmtmsg(DWORD dwFlags, LPCVOID pSrc, DWORD dwMsgId,
						  DWORD dwLangId, PWSTR pBuf, DWORD nSize,
						  va_list* vlArgs)
	{
		return FormatMessageW(dwFlags, pSrc, dwMsgId, dwLangId,
							  pBuf, nSize,vlArgs);
	}
#endif
 


// FUNCTION: sscpy.  Copies up to 'nMax' characters from pSrc to pDst.
// -----------------------------------------------------------------------------
// FUNCTION:  sscpy
//		static void sscpy(PSTR pDst, PCSTR pSrc, int nMax=0);
//		static void sscpy(PWSTR pDst, PCWSTR pSrc, int nMax=0);
//		static void sscpy(PWSTR pDst, PCSTR pSrc, int nMax=0);
//		static void sscpy(PSTR pDst, PCWSTR pSrc, int nMax=0);
//
// DESCRIPTION:
//		This function is very much (but not exactly) like strcpy.
//		strcpy equivalents.  These are not inline because they're too big.
//		When built as a DLL, we export them.  In previous versions of this
//		class, these were the equivalent of the static member
//		CStdString::CopyString with the destination and source args reversed.
//
//		These 3 overloads simplify copying one C-style string into another.
//		The strings must NOT overlap
//
//		"Character" is expressed in terms of the destination string, not
//		the source.  If no 'nMax' argument is supplied, then the number of
//		characters copied will be sslen(pSrc).  A NULL terminator will
//		also be added so pDst must actually be big enough to hold nMax+1
//		characters.  The return value is the number of characters copied,
//		not including the NULL terminator.
//
// PARAMETERS: 
//		pSrc - the string to be copied FROM.  May be a char based string, an
//			   MBCS string (in Win32 builds) or a wide string (wchar_t).
//		pSrc - the string to be copied TO.  Also may be either MBCS or wide
//		nMax - the maximum number of characters to be copied into szDest.  Note
//			   that this is expressed in whatever a "character" means to pDst.
//			   If pDst is a wchar_t type string than this will be the maximum
//			   number of wchar_ts that my be copied.  The pDst string must be
//			   large enough to hold least nMaxChars+1 characters.
//			   If the caller supplies no argument for nMax this is a signal to
//			   the routine to copy all the characters in pSrc, regardless of
//			   how long it is.
//
// RETURN VALUE: none
// -----------------------------------------------------------------------------
template <typename CT>
int sscpysame(CT* pDst, const CT* pSrc, int nMax=-1)
{
	int nSrcLen = NULL == pSrc ? 0 : sslen(pSrc);
	int nChars = ( nMax >= 0 ? SSMIN(nMax, nSrcLen) : nSrcLen );
//	memcpy(pDst, pSrc, nChars * sizeof(CT));
	NSP::char_traits<CT>().copy(pDst, pSrc, nChars * sizeof(CT));
	if ( nChars > 0 )
		pDst[nChars]	= '\0';

	return nChars;
}
inline int sscpy(PSTR pDst,  PCSTR pSrc, int nMax=-1)
{
	return sscpysame(pDst, pSrc, nMax);
}
inline int sscpy(PWSTR pDst, PCWSTR pSrc, int nMax=-1)
{
	return sscpysame(pDst, pSrc, nMax);
}
template<typename CT1, typename CT2>
inline int sscpydiff(CT1* pDst, const CT2* pSrc, int nMax=-1)
{
	int nSrcLen = NULL == pSrc ? 0 : sslen(pSrc);
	int nChars	= nMax >= 0 ? SSMIN(nMax, nSrcLen) : nSrcLen;
	StdCodeCvt(pDst, pSrc, nChars);
	if ( nChars > 0 )
		pDst[nChars]	= '\0';

	return nChars;
}
inline int sscpy(PWSTR pDst, PCSTR  pSrc, int nMax=-1)
{
	return sscpydiff(pDst, pSrc, nMax);
}
inline int sscpy(PSTR pDst, PCWSTR pSrc, int nMax=-1)
{
	return sscpydiff(pDst, pSrc, nMax);
}


//			Now we can define the template (finally!)
// =============================================================================
// TEMPLATE: CStdStr
//		template<typename CT> class CStdStr : public NSP::basic_string<CT>
//
// REMARKS:
//		This template derives from basic_string<CT> and adds some MFC CString-
//		like functionality
//
//		Basically, this is my attempt to make Standard C++ library strings as
//		easy to use as the MFC CString class.
//
//		Note that although this is a template, it makes the assumption that the
//		template argument (CT, the character type) is either char or wchar_t.  
// =============================================================================

#define CStdStr _SS	// avoid compiler warning 4786

#include "_defanwlib1ext.h"
template<typename CT>
class ANWLIB1_EXT CStdStr : public NSP::basic_string<CT>
{
	// Typedefs for shorter names.  Using these names also appears to help
	// us avoid some ambiguities that otherwise arise on some platforms

	typedef NSP::basic_string<CT>	MYBASE;	 // my base class (string or wstring)
	typedef CStdStr<CT>				MYTYPE;	 // myself
	typedef typename MYBASE::const_pointer	PCMYSTR; // PCSTR or PCWSTR 
	typedef typename MYBASE::pointer			PMYSTR;	 // PSTR or PWSTR
	typedef typename MYBASE::iterator		MYITER;  // my iterator type
	typedef typename MYBASE::const_iterator	MYCITER; // you get the idea...
	typedef typename MYBASE::size_type		MYSIZE;   
	typedef typename MYBASE::value_type		MYVAL; 
	typedef typename MYBASE::allocator_type	MYALLOC;
	

public:

	// constructors

							CStdStr();
							CStdStr(const CStdStr<CT>& str);
							CStdStr(const NSP::string& strA);
							CStdStr(const NSP::wstring& strW);
							CStdStr(PCMYSTR pT, MYSIZE n);
							CStdStr(PCSTR pA);
							CStdStr(PCWSTR pW);
							CStdStr(MYCITER first, MYCITER last);
							CStdStr(MYSIZE nSize,
									MYVAL ch,
									const MYALLOC& al=MYALLOC());
#ifdef SS_INC_COMDEF
							CStdStr(const _bstr_t& bstr);
#endif

	// assignment operators

			CStdStr<CT>&	operator=(const CStdStr<CT>& str);
			CStdStr<CT>&	operator=(const NSP::string& strA);	
			CStdStr<CT>&	operator=(const NSP::wstring& strW);
			CStdStr<CT>&	operator=(PCSTR pT);
			CStdStr<CT>&	operator=(PCWSTR pO);
			CStdStr<CT>&	operator=(CT t);
#ifdef SS_INC_COMDEF
			CStdStr<CT>&	operator=(const _bstr_t& bstr);
#endif

	// These overloads are also needed to fix the MSVC assignment bug (only for
	// MS' STL -- KB: Q172398). This bug was fixed in VC6 -- hence the macro
	// *** Thanks to Pete The Plumber for catching this one ***

#if ( defined(_MSC_VER) && ( _MSC_VER < 1200 ) ) || defined(SS_NO_REFCOUNT)
			CStdStr<CT>&	assign(const CStdStr<CT>& str);
			CStdStr<CT>&	assign(const CStdStr<CT>& str,
								   MYSIZE nStart,
								   MYSIZE nChars);
			CStdStr<CT>&	assign(const NSP::basic_string<CT>& str);
			CStdStr<CT>&	assign(const NSP::basic_string<CT>& str,
								   MYSIZE nStart,
								   MYSIZE nChars);
			CStdStr<CT>&	assign(const CT* pT, MYSIZE nChars);
			CStdStr<CT>&	assign(MYSIZE nChars, MYVAL val);
			CStdStr<CT>&	assign(const CT* pT);
			CStdStr<CT>&	assign(MYCITER iterFirst, MYCITER iterLast);
#endif

	// concatenation.
			CStdStr<CT>&	operator+=(const CStdStr<CT>& str);
			CStdStr<CT>&	operator+=(const NSP::string& str);
			CStdStr<CT>&	operator+=(const NSP::wstring& str);
			CStdStr<CT>&	operator+=(PCSTR pA);
			CStdStr<CT>&	operator+=(PCWSTR pW);
			CStdStr<CT>&	operator+=(CT t);
#ifdef SS_INC_COMDEF
			CStdStr<CT>&	operator+=(const _bstr_t& bstr);
#endif

	// addition operators -- global friend functions.

	friend	CStdStr<CT>	operator+(const CStdStr<CT>& str1,	const CStdStr<CT>& str2);
	friend	CStdStr<CT>	operator+(const CStdStr<CT>& str,	CT t);
	friend	CStdStr<CT>	operator+(const CStdStr<CT>& str,	PCSTR sz);
	friend	CStdStr<CT>	operator+(const CStdStr<CT>& str,	PCWSTR sz);
	friend	CStdStr<CT>	operator+(PCSTR pA,					const CStdStr<CT>& str);
	friend	CStdStr<CT>	operator+(PCWSTR pW,				const CStdStr<CT>& str);
#ifdef SS_INC_COMDEF
	friend	CStdStr<CT>	operator+(const _bstr_t& bstr,		const CStdStr<CT>& str);
	friend	CStdStr<CT>	operator+(const CStdStr<CT>& str,	const _bstr_t& bstr);
#endif

	// Functions that make us almost as easy to use as MFC's CString

#ifndef SS_ANSI
			bool			Load(UINT nId);							// load using resource id
#endif
			CStdStr<CT>&	Trim();									// remove whitespace on both sides
			CStdStr<CT>&	ToUpper();								// make uppercase
			CStdStr<CT>&	ToLower();								// make lowercase
			CStdStr<CT>&	Normalize();							// same as Trim().ToLower()
			CT*				Buffer(int nMinLen=-1);					// like CString::GetBuffer()
			CT*				BufferSet(int nLen);					// like CString::GetBufferSetLength
			void			BufferRel(int nNewLen=-1);				// like CString::ReleaseBuffer()
			bool			Equals(const CT* pT,					// case INsensitive comparison
								   bool bUseCase=false) const;
							operator const CT*() const;				// implicit cast to const TCHAR*
			void			Format(const CT* szFormat, ...);		// format using literal string
#ifndef SS_ANSI
			void			Format(UINT nId, ...);					// format using resource string ID
#endif
			void			FormatV(const CT* szFormat,				// format with va_list instead of ...
									va_list argList);	

	// CString facade functions -- make us a drop-in replacement for CString (almost).  

#ifndef SS_ANSI
			BSTR			AllocSysString() const;					// whip up a BSTR from our data
#endif
			int				Collate(PCMYSTR szThat) const;
			int				CollateNoCase(PCMYSTR szThat) const;
			int				CompareNoCase(PCMYSTR szThat) const;
			int				Delete(int nIdx, int nCount=1);
			void			Empty();								// erase  string and resize to 0
			int				Find(CT ch) const;						// find a character
			int				Find(PCMYSTR szSub) const;				// find a substring
			int				Find(CT ch, int nStart) const;			// find a character starting at nStart
			int				Find(PCMYSTR szSub, int nStart)const;	// Find a substring starting at nStart
			int				FindOneOf(PCMYSTR szCharSet) const;		// Find one character from szCharSet
#ifndef SS_ANSI
			void			FormatMessage(PCMYSTR szFormat, ...) throw(NSP::exception);
			void			FormatMessage(UINT nFormatId, ...) throw(NSP::exception);
#endif
			CT				GetAt(int nIdx) const;					// get character at nIdx
			CT*				GetBuffer(int nMinLen=-1);				// naked buffer of lenght at least nLen
			CT*				GetBufferSetLength(int nLen);			// naked buffer of length nlen
			int				GetLength() const;						// how many characters in the string?
			int				Insert(int nIdx, CT ch);				// insert character ch at index nIdx
			int				Insert(int nIdx, PCMYSTR sz);			// insert string sz at index nIdx
			bool			IsEmpty() const { return empty(); };	// is the string empty?
			CStdStr<CT>		Left(int nCount) const;					// get the left nCount characters
#ifndef SS_ANSI
			bool			LoadString(UINT nId);					// load from string resource
#endif
			void			MakeLower();							// make lower case
			void			MakeUpper();							// make upper case
			void			MakeReverse();							// reverse the string
			CStdStr<CT>		Mid(int nFirst) const;					// extract a substring 
			CStdStr<CT>		Mid(int nFirst, int nCount) const;		// extract a substring
			void			ReleaseBuffer(int nNewLen=-1);			// must call after GetBuffer
			int				Remove(CT ch);							// remove all occurrences of ch
			int				Replace(TCHAR chOld, TCHAR chNew);		// replace all chOld with chNew
			int				Replace(PCMYSTR szOld, PCMYSTR szNew);	// replace all szOld with chNew
			int				ReverseFind(CT ch);						// find starting from the back
			CStdStr<CT>		Right(int nCount) const;				// get the rightmost nCount characters
			void			SetAt(int nIndex, CT ch);				// set a character at zero-based index
			CStdStr<CT>		SpanExcluding(PCMYSTR szCharSet) const;	// extract start to any in szCharSet
			CStdStr<CT>		SpanIncluding(PCMYSTR szCharSet) const;	// extract start to any NOT in szCharSet
#ifndef SS_ANSI
			BSTR			SetSysString(BSTR* pbstr) const;		// reset a BSTR with our data
#endif
			CStdStr<CT>&	TrimLeft();								// remove whitespace on left
			CStdStr<CT>&	TrimLeft(CT tTrim);						// remove 'tTrim' charss on left
			CStdStr<CT>&	TrimLeft(PCMYSTR szTrimChars);			// remove chars in 'szTrimChars' on left
			CStdStr<CT>&	TrimRight();							// remove whitespace characters on right
			CStdStr<CT>&	TrimRight(CT tTrim);					// remove 'tTrim' characters on right
			CStdStr<CT>&	TrimRight(PCMYSTR szTrimChars);			// remove chars in 'szTrimChars' on right
#if !defined(UNICODE) && !defined(SS_ANSI)
			void			AnsiToOem();							// does anybody EVER call this?
			void			OemToAnsi();							// does anybody EVER call this?
#endif

			// I have intentionally not implemented the following CString
			// functions.   You cannot make them work without taking advantage
			// of implementation specific behavior.  However if you absolutely
			// MUST have them, uncomment out these lines for "sort-of-like"
			// their behavior.  You're on your own.

//			CT*				LockBuffer()	{ return Buffer(); }	// won't really lock the string
//			void			UnlockBuffer(); { }						// why have UnlockBuffer w/o LockBuffer?
//			void			FreeExtra();	{ resize(0); }			// not guaranteed to free anything

	// Array-indexing operators.  Required because we defined an implicit cast to operator const CT*
	// (Thanks to Julian Selman for pointing this out)

			CT&				operator[](int nIdx);
			const CT&		operator[](int nIdx) const;	
			CT&				operator[](unsigned int nIdx);
			const CT&		operator[](unsigned int nIdx) const;	

	// IStream related functions.  Useful in IPersistStream implementations
#ifdef SS_INC_COMDEF
			HRESULT			StreamSave(IStream* pStream) const;		// write to an IStream
			HRESULT			StreamLoad(IStream* pStream);			// read from an IStream;
			ULONG			StreamSize() const;						// # of bytes needed to StreamSave()
#endif
//	static	bool			Wildstrcmp (PCTSTR c_s, PCTSTR c_mask);

	// SetResourceHandle/GetResourceHandle.  In MFC builds, these map directly
	// to AfxSetResourceHandle and AfxGetResourceHandle.  In non-MFC builds they
	// point to a single static HINST so that those who call the member
	// functions that take resource IDs can provide an alternate HINST of a DLL
	// to search.  This is not exactly the list of HMODULES that MFC provides
	// but it's better than nothing.

	static	void			SetResourceHandle(HMODULE hNew);
	static	HMODULE			GetResourceHandle();
};

// shorthand conversion from PCTSTR to string resource ID
#define _TRES(pctstr) (LOWORD((DWORD)(pctstr)))	

/*/ CStdStr inline constructors
template<typename CT>
inline
CStdStr<CT>::CStdStr()
{
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(const CStdStr<CT>& str) : MYBASE(SSREF(str))
{
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(const NSP::string& str)
{
	ssasn(*this, SSREF(str));
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(const NSP::wstring& str)
{
	ssasn(*this, SSREF(str));
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(PCMYSTR pT, MYSIZE n) : MYBASE(pT, n)
{
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(PCSTR pA)
{
    if ( NULL != HIWORD(pA) )
        *this = pA;
#ifdef SS_ANSI
#else
    else if ( NULL != pA && !Load(_TRES(pA)) )
        TRACE(_T("Can't load string %u\n"), _TRES(pA));
#endif
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(PCWSTR pW)
{
    if ( NULL != HIWORD(pW) )
        *this = pW;
#ifdef SS_ANSI
#else
    else if ( NULL != pW && !Load(_TRES(pW)) )
        TRACE(_T("Can't load string %u\n"), _TRES(pW));
#endif
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(MYCITER first, MYCITER last)
	: MYBASE(first, last)
{
}

template<typename CT>
inline
CStdStr<CT>::CStdStr(MYSIZE nSize, MYVAL ch, const MYALLOC& al)
	: MYBASE(nSize, ch, al)
{
}

#ifdef SS_INC_COMDEF
	template<typename CT>
	inline CStdStr<CT>::CStdStr(const _bstr_t& bstr)
	{
		*this = static_cast<PCTSTR>(bstr);
	}
#endif


// CStdStr inline assignment operators -- the ssasn function now takes care
// of fixing  the MSVC assignment bug (see knowledge base article Q172398).
template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator=(const CStdStr<CT>& str)
{ 
	ssasn(*this, str); 
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator=(const NSP::string& str)
{
	ssasn(*this, str);
	return *this;
}

template<typename CT> 
inline
CStdStr<CT>& CStdStr<CT>::operator=(const NSP::wstring& str)
{
	ssasn(*this, str);
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator=(PCSTR pA)
{
	ssasn(*this, pA);
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator=(PCWSTR pW)
{
	ssasn(*this, pW);
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator=(CT t)
{
	Q172398(*this);
	MYBASE::assign(1, t);
	return *this;
}

#ifdef SS_INC_COMDEF
	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::operator=(const _bstr_t& bstr)
	{
		return operator=(static_cast<const CT*>(bstr));
	}
#endif



// These overloads are also needed to fix the MSVC assignment bug (KB: Q172398)
//  *** Thanks to Pete The Plumber for catching this one ***
// They also are compiled if you have explicitly turned off refcounting
#if ( defined(_MSC_VER) && ( _MSC_VER < 1200 ) ) || defined(SS_NO_REFCOUNT) 

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(const CStdStr<CT>& str)
	{
		ssasn(*this, str);
		return *this;
	}

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(const CStdStr<CT>& str,
									 MYSIZE nStart, MYSIZE nChars)
	{
		if ( this == &str )
		{
			CStdStr<CT> strTemp(str.c_str()+nStart, nChars);
			assign(strTemp);
		}
		else
		{
			Q172398(*this);
			MYBASE::assign(str.c_str()+nStart, nChars);
		}
		return *this;
	}

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(const NSP::basic_string<CT>& str)
	{
		ssasn(*this, str);
		return *this;
	}

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(const NSP::basic_string<CT>& str,
									 MYSIZE nStart, MYSIZE nChars)
	{
		if ( this == &str )	// watch out for assignment to self
		{
			CStdStr<CT> strTemp(str.c_str() + nStart, nChars);
			assign(strTemp);
		}
		else
		{
			Q172398(*this);
			MYBASE::assign(str.c_str()+nStart, nChars);
		}
		return *this;
	}

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(const CT* pC, MYSIZE nChars)
	{
		// Q172398 only fix -- erase before assigning, but not if we're assigning
		// from our own buffer

#if defined ( _MSC_VER ) && ( _MSC_VER < 1200 )
		if ( !empty() && ( pC < data() || pC > data() + capacity() ) )
			erase();
#endif
		Q172398(*this);
		MYBASE::assign(pC, nChars);
		return *this;
	}

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(MYSIZE nChars, MYVAL val)
	{
		Q172398(*this);
		MYBASE::assign(nChars, val);
		return *this;
	}

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(const CT* pT)
	{
		return assign(pT, CStdStr::traits_type::length(pT));
	}

	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::assign(MYCITER iterFirst,
									 MYCITER iterLast)
	{
#if defined ( _MSC_VER ) && ( _MSC_VER < 1200 ) 
		// Q172398 fix.  don't call erase() if we're assigning from ourself
		if ( iterFirst < begin() || iterFirst > begin() + size() )
			erase()
#endif
		replace(begin(), end(), iterFirst, iterLast);
		return *this;
	}
#endif

	
// CStdStr inline comparison (equality), defaults to case insensitive


// CStdStr inline comparison (equality), defaults to case insensitive
template<typename CT>
inline
bool CStdStr<CT>::Equals(const CT* pT, bool bUseCase) const
{	// get copy, THEN compare (thread safe)
	return  bUseCase ? compare(pT) == 0 : ssicmp(CStdStr<CT>(*this), pT) == 0;
} 

// CStdStr inline concatenation.
template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator+=(const MYTYPE& str)
{
	ssadd(*this, str);
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator+=(const NSP::string& str)
{
	ssadd(*this, str);
	return *this; 
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator+=(const NSP::wstring& str)
{
	ssadd(*this, str);
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator+=(PCSTR pA)
{
	ssadd(*this, pA);
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator+=(PCWSTR pW)
{
	ssadd(*this, pW);
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::operator+=(CT t)
{
	append(1, t);
	return *this;
}
#ifdef SS_INC_COMDEF	// if we have _bstr_t, define a += for it too.
	template<typename CT>
	inline
	CStdStr<CT>& CStdStr<CT>::operator+=(const _bstr_t& bstr)
	{
		return operator+=(static_cast<PCMYSTR>(bstr));
	}
#endif

// CStdStr friend addition functions defined as inline

template<typename CT>
inline
CStdStr<CT> operator+(const  CStdStr<CT>& str1, const  CStdStr<CT>& str2)
{
	CStdStr<CT> strRet(SSREF(str1));
	strRet.append(str2);
	return strRet;
}

template<typename CT>	
inline
CStdStr<CT> operator+(const  CStdStr<CT>& str, CT t)
{
	// this particular overload is needed for disabling reference counting
	// though it's only an issue from line 1 to line 2

	CStdStr<CT> strRet(SSREF(str));	// 1
	strRet.append(1, t);				// 2
	return strRet;
}

template<typename CT>
inline
CStdStr<CT> operator+(const  CStdStr<CT>& str, PCSTR pA)
{
	return CStdStr<CT>(str) + CStdStr<CT>(pA);
}

template<typename CT>
inline
CStdStr<CT> operator+(PCSTR pA, const  CStdStr<CT>& str)
{
	CStdStr<CT> strRet(pA);
	strRet.append(str);
	return strRet;
}

template<typename CT>
inline
CStdStr<CT> operator+(const CStdStr<CT>& str, PCWSTR pW)
{ 
	return CStdStr<CT>(SSREF(str)) + CStdStr<CT>(pW);
}

template<typename CT>
inline
CStdStr<CT> operator+(PCWSTR pW, const CStdStr<CT>& str)
{
	CStdStr<CT> strRet(pW);
	strRet.append(str);
	return strRet;
}

#ifdef SS_INC_COMDEF
	template<typename CT>
	inline
	CStdStr<CT> operator+(const _bstr_t& bstr, const CStdStr<CT>& str)
	{
		return static_cast<const CT*>(bstr) + str;
	}

	template<typename CT>
	inline
	CStdStr<CT> operator+(const CStdStr<CT>& str, const _bstr_t& bstr)
	{
		return str + static_cast<const CT*>(bstr);
	}
#endif

// CStdStr array indexing

template<typename CT>
inline
CT& CStdStr<CT>::operator[](int nIdx)
{
	return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
}

template<typename CT>
inline
const CT& CStdStr<CT>::operator[](int nIdx) const
{
	return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
}

template<typename CT>
inline
CT& CStdStr<CT>::operator[](unsigned int nIdx)
{
	return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
}

template<typename CT>
inline
const CT& CStdStr<CT>::operator[](unsigned int nIdx) const
{
	return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
}

template<typename CT>
inline
CStdStr<CT>::operator const CT*() const
{
	return c_str();
}

// CStdStr -- Direct access to character buffer.  In the MS' implementation, the
// at() function that we use here also calls _Freeze() providing us some measure
// of protection from multithreading problems associated with reference counting.
template<typename CT>
inline 
CT* CStdStr<CT>::Buffer(int nMinLen)
{
	if ( nMinLen > -1 )
		resize(static_cast<MYSIZE>(nMinLen));

	return empty() ? const_cast<CT*>(data()) : &(at(0));
}

template<typename CT>
inline
CT* CStdStr<CT>::BufferSet(int nLen)
{
	nLen = ( nLen > 0 ? nLen : 0 );
	if ( capacity() < 1 && nLen == 0 )
		resize(1);

	resize(static_cast<MYSIZE>(nLen));
	return const_cast<CT*>(data());
}

template<typename CT>
inline
void CStdStr<CT>::BufferRel(int nNewLen)
{
	resize(static_cast<MYSIZE>(nNewLen > -1 ? nNewLen : sslen(c_str())));
}


// This struct is used for TrimRight() and TrimLeft() function implementations.
template<typename CT>
struct NotSpace : public NSP::unary_function<CT, bool>
{
	inline bool operator() (CT tchar) { return !ssisp(tchar); }
};


template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::Trim()
{
	return TrimLeft().TrimRight();
}

template<typename CT>
CStdStr<CT>& CStdStr<CT>::TrimLeft()
{
	MYITER it = NSP::find_if(begin(), end(), NotSpace<CT>());
	if ( end() != it ) // avoid  assignment when possible
	{
		traits_type().move(&(*begin()), &(*it), (end() - it+1) * sizeof(CT));
		resize(end() - it);
	}

	return *this;
}

template<typename CT>
inline
CStdStr<CT>&  CStdStr<CT>::TrimLeft(CT tTrim)
{
	erase(0, find_first_not_of(tTrim));
	return *this;
}

template<typename CT>
inline
CStdStr<CT>&  CStdStr<CT>::TrimLeft(PCMYSTR szTrimChars)
{
	erase(0, find_first_not_of(szTrimChars));
	return *this;
}

template<typename CT>
CStdStr<CT>& CStdStr<CT>::TrimRight()
{
	reverse_iterator it = NSP::find_if(rbegin(), rend(), NotSpace<CT>());
	if ( rend() != it )
		erase(rend() - it);

	erase(it != rend() ? find_last_of(*it) + 1 : 0);
	return *this;
}

template<typename CT>
CStdStr<CT>&  CStdStr<CT>::TrimRight(CT tTrim)
{
	MYSIZE nIdx	= find_last_not_of(tTrim);
	if ( npos == nIdx )
	{
		erase();
	}
	else
	{
		// TODO: there's got to be a better, standard way to do this that
		// avoids calling length() (since  SGI's length() is slow)

		CT tSave	= at(nIdx);
		erase(nIdx, npos);	
		append(1, tSave);
	}
	return *this;
}

template<typename CT>
CStdStr<CT>&  CStdStr<CT>::TrimRight(PCMYSTR szTrimChars)
{
	MYSIZE nIdx	= find_last_not_of(szTrimChars);
	if ( npos == nIdx )
	{
		erase();
	}
	else
	{
		// TODO: there's got to be a better, standard way to do this that
		// avoids calling length() (since  SGI's length() is slow)

		CT tSave	= at(nIdx);
		erase(nIdx, npos);	
		append(1, tSave);
	}
	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::ToUpper()
{
	if ( !empty() )
		ssupr(Buffer(), size());

	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::ToLower()
{
	if ( !empty() )
		sslwr(Buffer(), size());

	return *this;
}

template<typename CT>
inline
CStdStr<CT>& CStdStr<CT>::Normalize()
{
	return Trim().ToLower();
}



#ifndef SS_ANSI
template<typename CT>
bool CStdStr<CT>::Load(UINT nId)
{
	bool bLoaded		= false;	// set to true of we succeed.

#ifdef _MFC_VER		// When in Rome...

	CString strRes;
	bLoaded				= FALSE != strRes.LoadString(nId);
	if ( bLoaded )
		*this			= strRes;

#else
	
	// Get the resource name and module handle

	PCTSTR szName		= MAKEINTRESOURCE((nId>>4)+1); // lifted from CString
	HMODULE hModule		= GetResourceHandle();
	DWORD dwSize		= 0;

	// No sense continuing if we can't find the resource

	HRSRC hrsrc			= ::FindResource(hModule, szName, RT_STRING);

	if ( NULL == hrsrc )
		TRACE(_T("Cannot find resource %d: 0x%X"), nId, ::GetLastError());
	else if ( 0 == (dwSize = ::SizeofResource(hModule, hrsrc) / sizeof(CT)) )
		TRACE(_T("Cannot get size of resource %d: 0x%X\n"), nId, GetLastError());
	else
	{
		bLoaded			= ( 0 != ssload(hModule, nId, Buffer(dwSize), dwSize) );
		ReleaseBuffer();
	}

#endif

	if ( !bLoaded )
		TRACE(_T("String not loaded 0x%X\n"), ::GetLastError());

	return bLoaded;
}
#endif

// -----------------------------------------------------------------------------
// FUNCTION:  CStdStr::Format
//		void _cdecl Formst(CStdStringA& PCSTR szFormat, ...)
//		void _cdecl Format(PCSTR szFormat);
//           
// DESCRIPTION:
//		This function does sprintf/wsprintf style formatting on CStdStringA
//		objects.  It looks a lot like MFC's CString::Format.  Some people might
//		even call this identical.  Unfortunately, these people have met with
//		untimely deaths (heh heh...)
//
// PARAMETERS: 
//		nId - ID of string resource holding the format string
//		szFormat - a PCSTR holding the format specifiers
//		argList - a va_list holding the arguments for the format specifiers.
//
// RETURN VALUE:  None.
// -----------------------------------------------------------------------------
// formatting (using wsprintf style formatting)
#ifndef SS_ANSI
template<typename CT>
void CStdStr<CT>::Format(UINT nId, ...)
{
	va_list argList;
	va_start(argList, nId);
	va_start(argList, nId);

	MYTYPE strFmt;
	if ( strFmt.Load(nId) )
		FormatV(strFmt, argList);

	va_end(argList);
}
#endif
template<typename CT>
void CStdStr<CT>::Format(const CT* szFormat, ...)
{
	va_list argList;
	va_start(argList, szFormat);
	FormatV(szFormat, argList);
	va_end(argList);
}

// -----------------------------------------------------------------------------
// FUNCTION:  CStdStr<CT>::FormatV
//		void FormatV(PCSTR szFormat, va_list, argList);
//           
// DESCRIPTION:
//		This function formats the string with sprintf style format-specs.  It
//		makes a general guess at required buffer size and then tries
//		successively larger buffers until it finds one big enough or a threshold
//		(MAX_FMT_TRIES) is exceeded.
//
// PARAMETERS: 
//		szFormat - a PCSTR holding the format of the output
//		argList - a Microsoft specific va_list for variable argument lists
//
// RETURN VALUE: 
// -----------------------------------------------------------------------------
#define MAX_FMT_TRIES		5	 // #of times we try if stack buffers not enough
#define FMT_BLOCK_SIZE		2048 // # of bytes above BUFSIZE_2ND_TRY each try
#define BUFSIZE_1ST	256
#define BUFSIZE_2ND 512
#define STD_BUF_SIZE		1024

template<typename CT>
void CStdStr<CT>::FormatV(const CT* szFormat, va_list argList)
{
#ifdef SS_ANSI

	int nLen	= sslen(szFormat) + STD_BUF_SIZE;
	ssvsprintf(GetBuffer(nLen), nLen-1, szFormat, argList);
	ReleaseBuffer();

#else

	// We're going to use the  _vsntprintf function, assuming FMT_BLOCK_SIZE
	// characters.

	int nTriesLeft			= MAX_FMT_TRIES-1;
	int nUsed				= -1;
	CT szBuf1[BUFSIZE_1ST];
	
	if ( (nUsed=ssnprintf(szBuf1, BUFSIZE_1ST-1, szFormat, argList)) >= 0 )
	{
		szBuf1[nUsed]		= '\0';
		*this				= szBuf1;
	}
	else
	{
		CT szBuf2[BUFSIZE_2ND];
		if ( (nUsed=ssnprintf(szBuf2, BUFSIZE_2ND-1, szFormat, argList))>=0)
		{
			szBuf2[nUsed]	= '\0';
			*this			= szBuf2;
		}
		else
		{
			int nChars		= BUFSIZE_2ND; 

			// Loop until we succeed or we have exhausted the number of tries

			do	
			{
				nChars		+= FMT_BLOCK_SIZE;	// # of TCHARS in the string

				// Now try the actual formatting. 

				nUsed		= ssnprintf(Buffer(nChars+1),
										nChars,
										szFormat,
										argList);

				if ( nUsed >= 0 )
					resize(nUsed);

			} while ( nUsed < 0 && --nTriesLeft > 0);
		}
	}
#endif
}


// CStdStr -- COM IStream helpers


#ifdef SS_INC_COMDEF

	#define SSSO_UNICODE	0x01	// the string is a wide string
	#define SSSO_COMPRESS	0x02	// the string is compressed

	typedef struct SSSHDR
	{
		BYTE	byCtrl;
		ULONG	nChars;
	} SSSHDR;	// as in "Standard String Stream Header"

	// -----------------------------------------------------------------------------
	// FUNCTION: CStdStr<CT>::StreamSize
	// REMARKS:
	// PARAMETERS:
	// RETURN VALUE:
	// -----------------------------------------------------------------------------
	template<typename CT>
	ULONG CStdStr<CT>::StreamSize() const
	{
		// Control header plus string
		ASSERT(size()*sizeof(CT) < 0xffffffffUL - sizeof(SSSHDR));
		return (size() * sizeof(CT)) + sizeof(SSSHDR);
	}

	// -----------------------------------------------------------------------------
	// FUNCTION: CStdStr<CT>::StreamSave
	// REMARKS:
	// PARAMETERS:
	// RETURN VALUE:
	// -----------------------------------------------------------------------------
	template<typename CT>
	HRESULT CStdStr<CT>::StreamSave(IStream* pStream) const
	{
		ASSERT(size()*sizeof(CT) < 0xffffffffUL - sizeof(SSSHDR));
		HRESULT hr		= E_FAIL;
		ASSERT(pStream != NULL);
		SSSHDR hdr;
		hdr.byCtrl		= sizeof(CT) == 2 ? SSSO_UNICODE : 0;
		hdr.nChars		= size();


		if ( FAILED(hr=pStream->Write(&hdr, sizeof(SSSHDR), NULL)) )
			TRACE(_T("StreamSave: Cannot write control header, ERR=0x%X\n"), hr);
		else if ( empty() )
			;		// nothing to write
		else if ( FAILED(hr=pStream->Write(c_str(), size()*sizeof(CT), NULL)) )
			TRACE(_T("StreamSave: Cannot write string to stream 0x%X\n"), hr);

		return hr;
	}

	// -----------------------------------------------------------------------------
	// FUNCTION: CStdStr<CT>::StreamLoad
	// REMARKS:
	//		This method loads the object from an IStream.
	// PARAMETERS:
	// RETURN VALUE:
	// -----------------------------------------------------------------------------
	template<typename CT>
	HRESULT CStdStr<CT>::StreamLoad(IStream* pStream)
	{
		ASSERT(pStream != NULL);
		SSSHDR hdr;
		HRESULT hr			= E_FAIL;

		if ( FAILED(hr=pStream->Read(&hdr, sizeof(SSSHDR), NULL)) )
		{
			TRACE(_T("StreamLoad: Unable to read control header, ERR=0x%X\n"), hr);
		}
		else if ( hdr.nChars > 0 )
		{
			ULONG nRead		= 0;
			PMYSTR pMyBuf	= BufferSet(hdr.nChars);

			// If our character size matches the character size of the string we're
			// trying to read, then we can read it directly into our buffer.
			// Otherwise, we have to read into an intermediate buffer and convert.
			
			if ( (hdr.byCtrl & SSSO_UNICODE) != 0 )
			{
				ULONG nBytes	= hdr.nChars * sizeof(wchar_t);
				if ( sizeof(CT) == sizeof(wchar_t) )
				{
					if ( FAILED(hr=pStream->Read(pMyBuf, nBytes, &nRead)) )
						TRACE(_T("StreamLoad: Cannot read string: 0x%X\n"), hr);
				}
				else
				{	
					PWSTR pBufW = reinterpret_cast<PWSTR>(_alloca((nBytes)+1));
					if ( FAILED(hr=pStream->Read(pBufW, nBytes, &nRead)) )
						TRACE(_T("StreamLoad: Cannot read string: 0x%X\n"), hr);
					else
						sscpy(pMyBuf, pBufW, hdr.nChars*sizeof(wchar_t));
				}
			}
			else
			{
				ULONG nBytes	= hdr.nChars * sizeof(char);
				if ( sizeof(CT) == sizeof(char) )
				{
					if ( FAILED(hr=pStream->Read(pMyBuf, nBytes, &nRead)) )
						TRACE(_T("StreamLoad: Cannot read string: 0x%X\n"), hr);
				}
				else
				{
					PSTR pBufA = reinterpret_cast<PSTR>(_alloca(nBytes));
					if ( FAILED(hr=pStream->Read(pBufA, hdr.nChars, &nRead)) )
						TRACE(_T("StreamLoad: Cannot read string: 0x%X\n"), hr);
					else
						sscpy(pMyBuf, pBufA, hdr.nChars);
				}
			}
		}
		else
		{
			this->erase();
		}
		return hr;
	}
#endif // #ifdef SS_INC_COMDEF


// -----------------------------------------------------------------------------
// CString Facade Functions:
//
// The following methods are intended to allow you to use this class as a
// drop-in replacement for CString.
// -----------------------------------------------------------------------------
#ifndef SS_ANSI
	template<typename CT>
	inline
	BSTR CStdStr<CT>::AllocSysString() const
	{
		ostring os;
		ssasn(os, *this);
		return ::SysAllocString(os.c_str());
	}
#endif

template<typename CT>
inline
int CStdStr<CT>::Collate(PCMYSTR szThat) const
{
	return sscoll(c_str(), szThat);
}

template<typename CT>
inline
int CStdStr<CT>::CollateNoCase(PCMYSTR szThat) const
{
	return ssicoll(c_str(), szThat);
}

template<typename CT>
inline
int CStdStr<CT>::CompareNoCase(PCMYSTR szThat)	const
{
	return ssicmp(c_str(), szThat);
}

template<typename CT>
inline
int CStdStr<CT>::Delete(int nIdx, int nCount)
{
	if ( nIdx < GetLength() )
		erase(static_cast<MYSIZE>(nIdx), static_cast<MYSIZE>(nCount));

	return GetLength();
}

template<typename CT>
inline
void CStdStr<CT>::Empty()
{
	erase();
}

template<typename CT>
inline
int CStdStr<CT>::Find(CT ch) const
{
	MYSIZE nIdx	= find_first_of(ch);
	return static_cast<int>(nIdx == npos ? -1 : nIdx);
}

template<typename CT>
inline
int CStdStr<CT>::Find(PCMYSTR szSub) const
{
	MYSIZE nIdx	= find(szSub);
	return static_cast<int>(nIdx == npos ? -1 : nIdx);
}

template<typename CT>
inline
int CStdStr<CT>::Find(CT ch, int nStart) const
{
	// must add 1 because Find excludes first character

	MYSIZE nIdx	= find_first_of(ch, static_cast<MYSIZE>(nStart+1));
	return static_cast<int>(nIdx == npos ? -1 : nIdx);
}

template<typename CT>
inline
int CStdStr<CT>::Find(PCMYSTR szSub, int nStart) const
{
	// must add 1 because Find excludes first character

	MYSIZE nIdx	= find(szSub, static_cast<MYSIZE>(nStart+1));
	return static_cast<int>(nIdx == npos ? -1 : nIdx);
}

template<typename CT>
inline
int CStdStr<CT>::FindOneOf(PCMYSTR szCharSet) const
{
	MYSIZE nIdx = find_first_of(szCharSet);
	return static_cast<int>(nIdx == npos ? -1 : nIdx);
}

#ifndef SS_ANSI
template<typename CT>
void CStdStr<CT>::FormatMessage(PCMYSTR szFormat, ...) throw(NSP::exception)
{
	va_list argList;
	va_start(argList, szFormat);
	PMYSTR szTemp;
	if ( ssfmtmsg(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
				   szFormat, 0, 0,
				   reinterpret_cast<PMYSTR>(&szTemp), 0, &argList) == 0 ||
		 szTemp == NULL )
	{
		throw NSP::runtime_error("out of memory");
	}
	*this = szTemp;
	LocalFree(szTemp);
	va_end(argList);
}

template<typename CT>
void CStdStr<CT>::FormatMessage(UINT nFormatId, ...) throw(NSP::exception)
{
	MYTYPE sFormat;
	VERIFY(sFormat.LoadString(nFormatId) != 0);
	va_list argList;
	va_start(argList, nFormatId);
	PMYSTR szTemp;
	if ( ssfmtmsg(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
				   sFormat, 0, 0,
				   reinterpret_cast<PMYSTR>(&szTemp), 0, &argList) == 0 ||
		szTemp == NULL)
	{
		throw NSP::runtime_error("out of memory");
	}
	*this = szTemp;
	LocalFree(szTemp);
	va_end(argList);
}
#endif

template<typename CT>
inline
CT CStdStr<CT>::GetAt(int nIdx) const
{
	return at(static_cast<MYSIZE>(nIdx));
}

template<typename CT>
inline
CT* CStdStr<CT>::GetBuffer(int nMinLen)
{
	return Buffer(nMinLen);
}

template<typename CT>
inline
CT* CStdStr<CT>::GetBufferSetLength(int nLen)
{
	return BufferSet(nLen);
}

// GetLength() -- MFC docs say this is the # of BYTES but
// in truth it is the number of CHARACTERs (chars or wchar_ts)
template<typename CT>
inline
int CStdStr<CT>::GetLength() const
{
	return static_cast<int>(length());
}

template<typename CT>
int CStdStr<CT>::Insert(int nIdx, CT ch)
{
	if ( static_cast<MYSIZE>(nIdx) > size() -1 )
		append(1, ch);
	else
		insert(static_cast<MYSIZE>(nIdx), 1, ch);

	return GetLength();
}

template<typename CT>
inline
int CStdStr<CT>::Insert(int nIdx, PCMYSTR sz)
{
	insert(static_cast<MYSIZE>(nIdx), sz);
	return GetLength();
}

template<typename CT> 
inline
CStdStr<CT> CStdStr<CT>::Left(int nCount) const
{
	return substr(0, static_cast<MYSIZE>(nCount)); 
}

#ifndef SS_ANSI
template<typename CT>
inline
bool CStdStr<CT>::LoadString(UINT nId)
{
	return this->Load(nId);
}
#endif
template<typename CT>
inline
void CStdStr<CT>::MakeLower()
{
	ToLower();
}

template<typename CT>
inline
void CStdStr<CT>::MakeReverse()
{
	NSP::reverse(begin(), end());
}

template<typename CT>
inline
void CStdStr<CT>::MakeUpper()
{ 
	ToUpper();
}

template<typename CT>
inline
CStdStr<CT> CStdStr<CT>::Mid(int nFirst ) const
{
	return substr(static_cast<MYSIZE>(nFirst));
}

template<typename CT>
inline
CStdStr<CT> CStdStr<CT>::Mid(int nFirst, int nCount) const
{
	return substr(static_cast<MYSIZE>(nFirst), static_cast<MYSIZE>(nCount));
}

template<typename CT>
inline
void CStdStr<CT>::ReleaseBuffer(int nNewLen)
{
	BufferRel(nNewLen);
}

template<typename CT>
int CStdStr<CT>::Remove(CT ch)
{
	MYSIZE nIdx		= 0;
	int nRemoved	= 0;
	while ( (nIdx=find_first_of(ch)) != npos )
	{
		erase(nIdx, 1);
		nRemoved++;
	}
	return nRemoved;
}

template<typename CT>
int CStdStr<CT>::Replace(TCHAR chOld, TCHAR chNew)
{
	int nReplaced	= 0;
	for ( MYITER iter=begin(); iter != end(); iter++ )
	{
		if ( *iter == chOld )
		{
			*iter = chNew;
			nReplaced++;
		}
	}
	return nReplaced;
}

template<typename CT>
int CStdStr<CT>::Replace(PCMYSTR szOld, PCMYSTR szNew)
{
	int nReplaced		= 0;
	MYSIZE nIdx			= 0;
	static const CT _C	= CT(0);
	MYSIZE nOldLen		= sslen(szOld);
	MYSIZE nNewLen		= sslen(szNew);
	PCMYSTR szRealNew	= szNew == NULL ? &_C : szNew;
	PCMYSTR szRealOld	= szOld == NULL ? &_C : szOld;
	while ( (nIdx=find(szRealOld, nIdx)) != npos )
	{
		replace(begin()+nIdx, begin()+nIdx+nOldLen, szRealNew);
		nReplaced++;
		nIdx += nNewLen;
	}
	return nReplaced;
}

template<typename CT>
inline
int CStdStr<CT>::ReverseFind(CT ch)
{
	MYSIZE nIdx	= find_last_of(ch);
	return static_cast<int>(nIdx == CStdStr<CT>::npos ? -1 : nIdx);
}

template<typename CT>
inline
CStdStr<CT> CStdStr<CT>::Right(int nCount) const
{
	nCount = SSMIN(nCount, static_cast<int>(size()));
	return substr(size()-static_cast<MYSIZE>(nCount));
}

template<typename CT>
inline
void CStdStr<CT>::SetAt(int nIndex, CT ch)
{
	ASSERT(size() > static_cast<MYSIZE>(nIndex));
	at(static_cast<MYSIZE>(nIndex))		= ch;
}

#ifndef SS_ANSI
	template<typename CT>
	BSTR CStdStr<CT>::SetSysString(BSTR* pbstr) const
	{
		ostring os;
		ssasn(os, *this);
		if ( !::SysReAllocStringLen(pbstr, os.c_str(), os.length()) )
			throw NSP::runtime_error("out of memory");

		ASSERT(*pbstr != NULL);
		return *pbstr;
	}
#endif
template<typename CT>
inline
CStdStr<CT> CStdStr<CT>::SpanExcluding(PCMYSTR szCharSet) const
{
	return Left(find_first_of(szCharSet));
}

template<typename CT>
inline
CStdStr<CT> CStdStr<CT>::SpanIncluding(PCMYSTR szCharSet) const
{
	return Left(find_first_not_of(szCharSet));
}//*/

#if !defined(UNICODE) && !defined(SS_ANSI)

	// CString's OemToAnsi and AnsiToOem functions are available only in Unicode
	// builds.  However since we're a template we also need a runtime check of
	// CT and a reinterpret_cast to account for the fact that CStdStringW gets
	// instantiated even in non-Unicode builds.


	template<typename CT>
	inline
	void CStdStr<CT>::AnsiToOem()
	{
		if ( sizeof(CT) == sizeof(char) && !empty() )
		{
			::CharToOem(reinterpret_cast<PCSTR>(c_str()),
						reinterpret_cast<PSTR>(Buffer()));
		}
		else
		{
			ASSERT(false);
		}
	}

	template<typename CT>
	inline
	void CStdStr<CT>::OemToAnsi()
	{
		if ( sizeof(CT) == sizeof(char) && !empty() )
		{
			::OemToChar(reinterpret_cast<PCSTR>(c_str()),
						reinterpret_cast<PSTR>(Buffer()));
		}
		else
		{
			ASSERT(false);
		}
	}

#endif

// =============================================================================
//						END OF CStdStr INLINE FUNCTION DEFINITIONS
// =============================================================================

//	Now typedef our class names based upon this humongous template

typedef CStdStr<char>		CStdStringA;	// a better NSP::string
typedef CStdStr<wchar_t>	CStdStringW;	// a better NSP::wstring
typedef CStdStr<OLECHAR>	CStdStringO;	// almost always CStdStringW

// our MFC-like resource handle

inline HMODULE& SSResourceHandle()
{
	static HMODULE hModuleSS	= GetModuleHandle(NULL);
	return hModuleSS;
}



// In MFC builds, define some global serialization operators
// Special operators that allow us to serialize CStdStrings to CArchives.
// Note that we use an intermediate CString object in order to ensure that
// we use the exact same format.

#ifdef _MFC_VER
	inline CArchive& AFXAPI operator<<(CArchive& ar, const CStdStringA& strA)
	{
		CString strTemp	= strA;
		return ar << strTemp;
	}
	inline CArchive& AFXAPI operator<<(CArchive& ar, const CStdStringW& strW)
	{
		CString strTemp	= strW;
		return ar << strTemp;
	}

	inline CArchive& AFXAPI operator>>(CArchive& ar, CStdStringA& strA)
	{
		CString strTemp;
		ar >> strTemp;
		strA = strTemp;
		return ar;
	}
	inline CArchive& AFXAPI operator>>(CArchive& ar, CStdStringW& strW)
	{
		CString strTemp;
		ar >> strTemp;
		strW = strTemp;
		return ar;
	}
/*
	template<typename CT>
	inline void CStdStr<CT>::SetResourceHandle(HMODULE hNew)
	{
		AfxSetResourceHandle(hNew);
	}
	template<typename CT> inline HMODULE CStdStr<CT>::GetResourceHandle()
	{
		return AfxGetResourceHandle();
	}//*/
#else
	template<typename CT>
	inline void CStdStr<CT>::SetResourceHandle(HMODULE hNew)
	{
		SSResourceHandle() = hNew;
	}
	template<typename CT>
	inline HMODULE CStdStr<CT>::GetResourceHandle()
	{
		return SSResourceHandle();
	}
#endif

// WUSysMessage -- return the system string corresponding to a system error or
// HRESULT value.
#define SS_DEFLANGID MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)

// -----------------------------------------------------------------------------
// GLOBAL FUNCTION:  WUFormat
//		CStdStringA WUFormat(UINT nId, ...);
//		CStdStringA WUFormat(PCSTR szFormat, ...);
//
// REMARKS:
//		This function allows the caller for format and return a CStdStringA
//		object with a single line of code.
// -----------------------------------------------------------------------------
#ifdef SS_ANSI
#else
	inline CStdStringA WUFormatA(UINT nId, ...)
	{
		va_list argList;
		va_start(argList, nId);

		CStdStringA strFmt;
		CStdStringA strOut;
		if ( strFmt.Load(nId) )
			strOut.FormatV(strFmt, argList);

		va_end(argList);
		return strOut;
	}
	inline CStdStringA WUFormatA(PCSTR szFormat, ...)
	{
		va_list argList;
		va_start(argList, szFormat);
		CStdStringA strOut;
		strOut.FormatV(szFormat, argList);
		va_end(argList);
		return strOut;
	}

	inline CStdStringW WUFormatW(UINT nId, ...)
	{
		va_list argList;
		va_start(argList, nId);

		CStdStringW strFmt;
		CStdStringW strOut;
		if ( strFmt.Load(nId) )
			strOut.FormatV(strFmt, argList);

		va_end(argList);
		return strOut;
	}
	inline CStdStringW WUFormatW(PCWSTR szwFormat, ...)
	{
		va_list argList;
		va_start(argList, szwFormat);
		CStdStringW strOut;
		strOut.FormatV(szwFormat, argList);
		va_end(argList);
		return strOut;
	}
#endif // #ifdef SS_ANSI

#ifdef SS_ANSI
#else
	// -------------------------------------------------------------------------
	// FUNCTION: WUSysMessage
	//	 CStdStringA WUSysMessageA(DWORD dwError, DWORD dwLangId=SS_DEFLANGID);
	//	 CStdStringW WUSysMessageW(DWORD dwError, DWORD dwLangId=SS_DEFLANGID);
	//           
	// DESCRIPTION:
	//	 This function simplifies the process of obtaining a string equivalent
	//	 of a system error code returned from GetLastError().  You simply
	//	 supply the value returned by GetLastError() to this function and the
	//	 corresponding system string is returned in the form of a CStdStringA.
	//
	// PARAMETERS: 
	//	 dwError - a DWORD value representing the error code to be translated
	//	 dwLangId - the language id to use.  defaults to english.
	//
	// RETURN VALUE: 
	//	 a CStdStringA equivalent of the error code.  Currently, this function
	//	 only returns either English of the system default language strings.  
	// -------------------------------------------------------------------------
	#define SS_DEFLANGID MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)
	inline CStdStringA WUSysMessageA(DWORD dwError, DWORD dwLangId=SS_DEFLANGID)
	{
		CHAR szBuf[512];

		if ( 0 != ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
								   dwLangId, szBuf, 511, NULL) )
			return WUFormatA("%s (0x%X)", szBuf, dwError);
		else
 			return WUFormatA("Unknown error (0x%X)", dwError);
	}
	inline CStdStringW WUSysMessageW(DWORD dwError, DWORD dwLangId=SS_DEFLANGID)
	{
		WCHAR szBuf[512];

		if ( 0 != ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
								   dwLangId, szBuf, 511, NULL) )
			return WUFormatW(L"%s (0x%X)", szBuf, dwError);
		else
 			return WUFormatW(L"Unknown error (0x%X)", dwError);
	}
#endif

// Define TCHAR based friendly names for some of these functions

#ifdef UNICODE
	#define CStdString				CStdStringW
	#define WUSysMessage			WUSysMessageW
	#define WUFormat				WUFormatW
#else
	#define CStdString				CStdStringA
	#define WUSysMessage			WUSysMessageA
	#define WUFormat				WUFormatA
#endif


#define WUSysMsg					WUSysMessage
#define WUFmtA						WUFormatA
#define	WUFmtW						WUFormatW
#define WUFmt						WUFormat
#define WULastErrMsg()				WUSysMessage(::GetLastError())


// -----------------------------------------------------------------------------
// FUNCTIONAL COMPARATORS:
// REMARKS:
//		These structs are derived from the NSP::binary_function template.  They
//		give us functional classes (which may be used in Standard C++ Library
//		collections and algorithms) that perform case-insensitive comparisons of
//		CStdString objects.  This is useful for maps in which the key may be the
//		 proper string but in the wrong case.
// -----------------------------------------------------------------------------

#define StdStringLessNoCaseW		SSLNCW		// avoid compiler warning 4786
#define StdStringEqualsNoCaseW		SSENCW		
#define StdStringLessNoCaseA		SSLNCA		
#define StdStringEqualsNoCaseA		SSENCA		

#ifdef UNICODE
	#define StdStringLessNoCase		SSLNCW		
	#define StdStringEqualsNoCase	SSENCW		
#else
	#define StdStringLessNoCase		SSLNCA		
	#define StdStringEqualsNoCase	SSENCA		
#endif

// -----------------------------------------------------------------------------
// FUNCTION: 
// REMARKS:
// PARAMETERS:
// RETURN VALUE:
// -----------------------------------------------------------------------------
struct StdStringLessNoCaseW
	: NSP::binary_function<CStdStringW, CStdStringW, bool>
{
	inline
	bool operator()(const CStdStringW& sLeft, const CStdStringW& sRight) const
	{ return ssicmp(sLeft.c_str(), sRight.c_str()) < 0; }
};
struct StdStringEqualsNoCaseW
	: NSP::binary_function<CStdStringW, CStdStringW, bool>
{
	inline
	bool operator()(const CStdStringW& sLeft, const CStdStringW& sRight) const
	{ return ssicmp(sLeft.c_str(), sRight.c_str()) == 0; }
};
struct StdStringLessNoCaseA
	: NSP::binary_function<CStdStringA, CStdStringA, bool>
{
	inline
	bool operator()(const CStdStringA& sLeft, const CStdStringA& sRight) const
	{ return ssicmp(sLeft.c_str(), sRight.c_str()) < 0; }
};
struct StdStringEqualsNoCaseA
	: NSP::binary_function<CStdStringA, CStdStringA, bool>
{
	inline
	bool operator()(const CStdStringA& sLeft, const CStdStringA& sRight) const
	{ return ssicmp(sLeft.c_str(), sRight.c_str()) == 0; }
};

// If we had to define our own version of TRACE above, get rid of it now

#ifdef TRACE_DEFINED_HERE
	#undef TRACE
	#undef TRACE_DEFINED_HERE
#endif





#endif	// #ifndef _STDSTRING_H_