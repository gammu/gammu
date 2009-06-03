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

#if !defined(GLIB_MAJOR_VERSION) && !defined(USE_GLIB)
/**
 * gboolean definition, compatible with glib.
 */
typedef int gboolean;
#ifndef FALSE
#define	FALSE	(0)
#endif
#ifndef TRUE
#define	TRUE	(!FALSE)
#endif
#endif

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
