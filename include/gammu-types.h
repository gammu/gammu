/**
 * \file gammu-types.h
 * \author Michal Čihař
 * 
 * Various types definitions
 */
#ifndef __gammu_types_h
#define __gammu_types_h

/**
 * \defgroup Types Types
 * Definition of various useful types.
 */

/* 
 * Need to be hidden behind ifndef, because some other code might 
 * define this also
 */

#ifndef bool
/**
 * Boolean type.
 */
#  define bool int
#endif

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
