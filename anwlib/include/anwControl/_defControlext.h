#ifndef _DEFCONTROLEXT_H
#define _DEFCONTROLEXT_H

#ifndef CONTROL_EXT
#ifdef COMPILING_CONTROL
#define CONTROL_EXT __declspec( dllexport )
#else
#define CONTROL_EXT __declspec( dllimport )
#endif
#endif

#endif //_DEFCONTROLEXT_H