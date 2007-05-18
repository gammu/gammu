/**
 * \file gammu-types.h
 * \author Michal Čihař
 * 
 * Various types definitions
 */
#ifndef __gammu_types_h
#define __gammu_types_h

/**
 * \defgroup Type Types
 * Definition of various useful types.
 */

/* 
 * Need to be hidden behind ifndef, because some other code might 
 * define this also
 */

#ifndef __cplusplus

#  ifndef false
/**
 * False value for \ref bool.
 *
 * \ingroup Type
 */
#    define false (0)
#  endif
#  ifndef true
/**
 * True value for \ref bool.
 *
 * \ingroup Type
 */
#    define true (1)
#  endif
#  ifndef bool
/**
 * Boolean type.
 *
 * \ingroup Type
 */
#    define bool int
#  endif
#endif				/* __cplusplus */

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
