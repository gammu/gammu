
#ifndef _CFGREADER_H
#define _CFGREADER_H

/* Structure definitions */

/* A linked list of key/value pairs */

typedef struct _CFG_Entry CFG_Entry;

struct _CFG_Entry {
	CFG_Entry 	*next, *prev;
	char 		*key;
        char 		*value;
};

typedef struct _CFG_Header CFG_Header;

struct _CFG_Header {
        CFG_Header 	*next, *prev;
        CFG_Entry 	*entries;
        char		*section;
};

/* Function prototypes */

CFG_Header 	*CFG_ReadFile(char *filename);
char            *CFG_Get(CFG_Header *cfg, char *section, char *key);
char            *CFG_Set(CFG_Header *cfg, char *section, char *key, char *value);
CFG_Entry 	*CFG_FindLastSectionEntry(CFG_Header *file_info, char *section);

#endif /* _CFGREADER_H */
