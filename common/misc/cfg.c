
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "../../cfg/config.h"
#include "misc.h"
#include "cfg.h"

/* Read configuration information from a ".INI" style file */
CFG_Header *CFG_ReadFile(char *filename)
{
	FILE			*handle;
        char			*line;
        char			*buf;
        CFG_Header 		*cfg_info = NULL, *cfg_head = NULL;

        /* Error check */
        if (filename == NULL) return NULL;

        /* Initialisation */
        if ((buf = (char *)malloc(255)) == NULL) return NULL;
    
        /* Open file */
        if ((handle = fopen(filename, "r")) == NULL) {
//                dprintf("CFG file - error opening \"%s\" file: %s\n", filename, strerror(errno));
                free(buf);
        	return NULL;
        } else {
//                dprintf("CFG file - opened file \"%s\"\n", filename );
	}

        /* Iterate over lines in the file */
        while (fgets(buf, 255, handle) != NULL) {

                line = buf;

                /* Strip leading, trailing whitespace */
                while(isspace((int) *line))
                        line++;

                while((strlen(line) > 0) && isspace((int) line[strlen(line) - 1]))
                        line[strlen(line) - 1] = '\0';
        
                /* Ignore blank lines and comments */
                if ((*line == '\n') || (*line == '\0') || (*line == '#'))
                        continue;

                /* Look for "headings" enclosed in square brackets */
                if ((line[0] == '[') && (line[strlen(line) - 1] == ']')) {
                        CFG_Header *heading;

                        /* Allocate new heading entry */
                        if ((heading = (CFG_Header *)malloc(sizeof(*heading))) == NULL) {
                                free(buf);
                                return NULL;
                        }

                        /* Fill in fields */
                        memset(heading, '\0', sizeof(*heading));
            
                        line++;
                        line[strlen(line) - 1] = '\0';

                        /* FIXME: strdup is not ANSI C compliant. */
                        heading->section = strdup(line);

                        /* Add to tail of list  */
                        heading->prev = cfg_info;

                        if (cfg_info != NULL) {
                                cfg_info->next = heading;
                        } else {
                                /* Store copy of head of list for return value */
                                cfg_head = heading;
                        }

                        cfg_info = heading;

//			dprintf( "CFG file - added new section \"%s\"\n", heading->section);

                        /* Go on to next line */
                        continue;
                }

                /* Process key/value line */
                if ((strchr(line, '=') != NULL) && cfg_info != NULL) {
                        CFG_Entry 	*entry;
                        char 		*value;

                        /* Allocate new entry */
                        if ((entry = (CFG_Entry *)malloc(sizeof(*entry))) == NULL) {
                                free(buf);
                                return NULL;
                        }

                        /* Fill in fields */
                        memset(entry, '\0', sizeof(*entry));

                        value = strchr(line, '=');
                        *value = '\0';                /* Split string */
                        value++;
            
                        while(isspace((int) *value)) {      /* Remove leading white */
                                value++;
                        }

                        /* FIXME: strdup is not ANSI C compliant. */
                        entry->value = strdup(value);

                        while((strlen(line) > 0) && isspace((int) line[strlen(line) - 1])) {
                                line[strlen(line) - 1] = '\0';  /* Remove trailing white */
                        }

                        /* FIXME: strdup is not ANSI C compliant. */
                        entry->key = strdup(line);

                        /* Add to head of list */
                        entry->next = cfg_info->entries;

                        if (cfg_info->entries != NULL) {
                                cfg_info->entries->prev = entry;
                        }

                        cfg_info->entries = entry;

/*                        dprintf("CFG file - adding key/value \"%s/%s\"\n", entry->key, entry->value); */

                        /* Go on to next line */
                        continue;
                }

                /* Line not part of any heading */
//                dprintf("CFG file - orphaned line: \"%s\"\n", line);
        }

        free(buf);
	
        /* Return pointer to configuration information */
        return cfg_head;
}

/* 
 * Find the value of a key in a config file.  Return value associated
 * with key or NULL if no such key exists. 
 */
char *CFG_Get(CFG_Header *cfg, char *section, char *key)
{
        CFG_Header *h;
        CFG_Entry *e;

        if ((cfg == NULL) || (section == NULL) || (key == NULL)) {
                return NULL;
        }

        /* Search for section name */
        for (h = cfg; h != NULL; h = h->next) {
                if (strcmp(section, h->section) == 0) {
                        /* Search for key within section */
                        for (e = h->entries; e != NULL; e = e->next) {
                                if (strcmp(key, e->key) == 0) {
                                        /* Found! */
                                        return e->value;
                                }
                        }
                }
        }
        /* Key not found in section */
        return NULL;
}

/*  Set the value of a key in a config file.  Return the new value if
 *  the section/key can be found, else return NULL.
 */
char *CFG_Set(CFG_Header *cfg, char *section, char *key, char *value)
{
        CFG_Header *h;
        CFG_Entry *e;

        if ((cfg == NULL) || (section == NULL) || (key == NULL) || 
            (value == NULL)) {
                return NULL;
        }

        /* Search for section name */
        for (h = cfg; h != NULL; h = h->next) {
                if (strcmp(section, h->section) == 0) {
                        /* Search for key within section */
                        for (e = h->entries; e != NULL; e = e->next) {
                                if ((e->key != NULL) && strcmp(key, e->key) == 0) {
                                        /* Found - set value */
                                        free(e->key);
                                        /* FIXME: strdup is not ANSI C compliant. */
                                        e->key = strdup(value);
                                        return e->value;
                                }
                        }
                }
        }
        /* Key not found in section */
        return NULL;    
}

/* Return last value in specified section */
CFG_Entry *CFG_FindLastSectionEntry(CFG_Header *file_info, char *section)
{
	CFG_Header 	*h;
	CFG_Entry	*e;

	e = NULL;
	/* First find our section */
        for (h = file_info; h != NULL; h = h->next) {
		if (strcmp(section, h->section) == 0) {
			e = h->entries;
			break;
		}
	}
	/* Goes into last value in section */
	while (1) {
		if (e == NULL) break;
		if (e->next != NULL) {
			e = e->next;
		} else break;
	}
	return e;
}
