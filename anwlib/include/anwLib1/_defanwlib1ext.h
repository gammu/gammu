#ifndef _DEFANWLIB1_H
#define _DEFANWLIB1_H

#ifndef ANWLIB1_EXT
#ifdef COMPILING_ANWLIB1
#define ANWLIB1_EXT __declspec( dllexport )
#else
#define ANWLIB1_EXT __declspec( dllimport )
#endif
#endif

#endif //_DEFANWLIB1_H