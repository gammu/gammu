/*

  $Id: cfgreader.h,v 1.4 2000/12/19 16:18:19 pkot Exp $
  
  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for config file reader.

*/

#ifndef _CFGREADER_H
#define _CFGREADER_H

#ifndef WIN32
  #include "config.h"
#endif

/* Structure definitions */

/* A linked list of key/value pairs */

struct CFG_Entry {
        struct CFG_Entry *next, *prev;
        char *key;
        char *value;
};

struct CFG_Header {
        struct CFG_Header *next, *prev;
        struct CFG_Entry *entries;
        char *section;
};

/* Function prototypes */

struct CFG_Header *CFG_ReadFile(char *filename);
char              *CFG_Get(struct CFG_Header *cfg, char *section, char *key);
char              *CFG_Set(struct CFG_Header *cfg, char *section, char *key, 
                           char *value);
int                CFG_WriteFile(struct CFG_Header *cfg, char *filename);

int 		  CFG_ReadConfig(char **model, char **port, char **initlength,
                                 char **connection, char **bindir, char **synchronizetime,
                                 bool isgnokiid);

struct CFG_Header *CFG_FindGnokiirc();

#endif /* _CFGREADER_H */
