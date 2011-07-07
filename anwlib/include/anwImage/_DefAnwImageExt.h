#ifndef _DEFANWIMAGEEXT_H
#define _DEFANWIMAGEEXT_H

#ifndef ANWIMAGE_EXT
#ifdef COMPILING_ANWIMAGE
#define ANWIMAGE_EXT __declspec( dllexport )
#else
#define ANWIMAGE_EXT __declspec( dllimport )
#endif
#endif

#endif //_DEFANWIMAGEEXT_H