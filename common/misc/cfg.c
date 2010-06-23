
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifndef __OpenBSD__
#  include <wchar.h>
#  include <wctype.h>
#endif

#include "../../cfg/config.h"
#include "cfg.h"
#include "coding.h"
#include "misc.h"

/* Read configuration information from a ".INI" style file */
CFG_Header *CFG_ReadFile(char *filename, bool Unicode)
{
	FILE			*handle;
        unsigned char		*line,*buf,ch;
        CFG_Header 		*cfg_info = NULL, *cfg_head = NULL;
	int			i,pos;
	bool			process,FFEEUnicode=false,firstread=true;

        /* Error check */
        if (filename == NULL) return NULL;

        /* Initialisation */
        if ((buf = (char *)malloc(1400)) == NULL) return NULL;
    
        /* Open file */
        if ((handle = fopen(filename, "rb")) == NULL) {
//                dprintf("CFG file - error opening \"%s\" file: %s\n", filename, strerror(errno));
                free(buf);
        	return NULL;
        } else {
//                dprintf("CFG file - opened file \"%s\"\n", filename );
	}

        /* Iterate over lines in the file */
        while (1) {
		if (Unicode) {
			/* Here is my own version of fgetws */
			pos = 0;
			while (1) {
				if (fread(buf+pos,1,2,handle) != 2) {
					buf[pos]   = 0;
					buf[pos+1] = 0;
					break;
				}
				if (firstread) {
					FFEEUnicode = false;
					firstread   = false;
					if (buf[0] == 0xFF && buf[1] == 0xFE) {
						FFEEUnicode = true;
						continue;
					}
					if (buf[0] == 0xFE && buf[1] == 0xFF) {
						continue;
					}
				}
				if (FFEEUnicode) {
					ch 	   = buf[pos];
					buf[pos]   = buf[pos+1];
					buf[pos+1] = ch;
				}
				pos+=2;
				buf[pos]   = 0;
				buf[pos+1] = 0;
				if (buf[pos-2] == 0x00 && buf[pos-1] == 0x0a) break;
				if (buf[pos-2] == 0x00 && buf[pos-1] == 0x0d) break;
				if (pos == 1400) break;
			}
			if (pos == 0) break;
		} else {
			/* Here is my own version of fgets */
			pos = 0;
			while (1) {
				if (fread(buf+pos,1,1,handle) != 1) {
					buf[pos] = 0;
					break;
				}
				pos++;
				buf[pos] = 0;
				if (buf[pos-1] == 0x0a) break;
				if (buf[pos-1] == 0x0d) break;
				if (pos == 1400) break;
			}
			if (pos == 0) break;
		}

                line    = buf;
		process = false;

		if (Unicode) {
	                /* Strip leading, trailing whitespace */
	                while(myiswspace(line)) line+=2;

			while(1) {
				i = strlen(DecodeUnicodeString(line))*2;
				if (i>0 && myiswspace(line+i-2))
				{
					line[i - 2] = '\0';
		                        line[i - 1] = '\0';
				} else break;
			}
        
	                /* Ignore blank lines and comments */
	                if ((*line == 0 && *(line+1) == 0x0a) ||
			    (*line == 0 && *(line+1) == 0x0d) ||
			    (*line == 0 && *(line+1) == 0)    ||
			    (*line == 0 && *(line+1) == '#'))
	                        continue;

	                /* Look for "headings" enclosed in square brackets */
	                if ((line[0] == 0 && line[1] == '[') &&
			    (line[strlen(DecodeUnicodeString(line))*2 - 2] == 0 &&
			     line[strlen(DecodeUnicodeString(line))*2 - 1] == ']')) 
				process = true;
		} else {
	                /* Strip leading, trailing whitespace */
	                while(isspace((int) *line)) line++;

        	        while((strlen(line) > 0) && isspace((int) line[strlen(line) - 1]))
	                        line[strlen(line) - 1] = '\0';
        
	                /* Ignore blank lines and comments */
	                if ((*line == '\n') || (*line == '\0') || (*line == '#'))
	                        continue;

	                /* Look for "headings" enclosed in square brackets */
	                if ((line[0] == '[') && (line[strlen(line) - 1] == ']')) process = true;
		}
		if (process) {
                        CFG_Header *heading;

                        /* Allocate new heading entry */
                        if ((heading = (CFG_Header *)malloc(sizeof(*heading))) == NULL) {
                                free(buf);
                                return NULL;
                        }

                        /* Fill in fields */
                        memset(heading, '\0', sizeof(*heading));

			if (Unicode) {            
	                        line+=2;
				i = strlen(DecodeUnicodeString(line))*2;
	                        line[i - 2] = '\0';
	                        line[i - 1] = '\0';

				heading->section = (char *)malloc(strlen(DecodeUnicodeString(line))*2+2);
				memcpy(heading->section,line,strlen(DecodeUnicodeString(line))*2+2);
			} else {
	                        line++;
	                        line[strlen(line) - 1] = '\0';

				heading->section = (char *)malloc(strlen(line)+1);
				memcpy(heading->section,line,strlen(line)+1);
			}

                        /* Add to tail of list  */
                        heading->prev = cfg_info;

                        if (cfg_info != NULL) {
                                cfg_info->next = heading;
                        } else {
                                /* Store copy of head of list for return value */
                                cfg_head = heading;
                        }

                        cfg_info = heading;

//			if (Unicode) dprintf( "CFG file - added new section \"%s\"\n", DecodeUnicodeString(heading->section));
//			printf( "CFG file - added new section \"%s\"\n", heading->section);

                        /* Go on to next line */
                        continue;
                }

                /* Process key/value line */
		process = false;
		if (Unicode) {
			if ((strchr(DecodeUnicodeString(line), '=') != NULL) && cfg_info != NULL) process = true;
		} else {
			if ((strchr(line, '=') != NULL) && cfg_info != NULL) process = true;
		}
                if (process) {
                        CFG_Entry 	*entry;
                        char 		*value;

                        /* Allocate new entry */
                        if ((entry = (CFG_Entry *)malloc(sizeof(*entry))) == NULL) {
                                free(buf);
                                return NULL;
                        }

                        /* Fill in fields */
                        memset(entry, '\0', sizeof(*entry));

			if (Unicode) {
				value = line;
				while(1) {
					if (*(value) == 0 && *(value+1) == '=') break;
					value += 2;
				}
	                        *(value) = 0;                /* Split string */
				*(value+1) = 0;
	                        value+=2;
            
				/* Remove leading white */
	                        while(myiswspace(value)) value+=2;

				entry->value = (char *)malloc(strlen(DecodeUnicodeString(value))*2+2);
				memcpy(entry->value,value,strlen(DecodeUnicodeString(value))*2+2);

				while(1) {
					i = strlen(DecodeUnicodeString(line))*2;
					if (i>0 && myiswspace(line+i-2))
					{
						line[i - 2] = '\0';
			                        line[i - 1] = '\0';
					} else break;
				}

				entry->key = (char *)malloc(strlen(DecodeUnicodeString(line))*2+2);
				memcpy(entry->key,line,strlen(DecodeUnicodeString(line))*2+2);
			} else {
	                        value = strchr(line, '=');
	                        *value = '\0';                /* Split string */
	                        value++;
            
	                        while(isspace((int) *value)) {      /* Remove leading white */
	                                value++;
	                        }

				entry->value = (char *)malloc(strlen(value)+1);
				memcpy(entry->value,value,strlen(value)+1);

	                        while((strlen(line) > 0) && isspace((int) line[strlen(line) - 1])) {
	                                line[strlen(line) - 1] = '\0';  /* Remove trailing white */
	                        }

				entry->key = (char *)malloc(strlen(line)+1);
				memcpy(entry->key,line,strlen(line)+1);
			}

                        /* Add to head of list */
                        entry->next = cfg_info->entries;

                        if (cfg_info->entries != NULL) cfg_info->entries->prev = entry;

                        cfg_info->entries = entry;

//			if (Unicode) {
//			    dprintf("CFG file - adding key/value \"%s/",DecodeUnicodeString(entry->key));
//			    dprintf("%s\"\n", DecodeUnicodeString(entry->value));
//			}

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
unsigned char *CFG_Get(CFG_Header *cfg, unsigned char *section, unsigned char *key, bool Unicode)
{
        CFG_Header *h;
        CFG_Entry  *e;

        if ((cfg == NULL) || (section == NULL) || (key == NULL)) {
                return NULL;
        }

	if (Unicode) {
//		printf("searching for %s",DecodeUnicodeString(section));
//		printf("/%s\n",DecodeUnicodeString(key));
	        /* Search for section name */
	        for (h = cfg; h != NULL; h = h->next) {
	                if (mywstrncasecmp(section, h->section, 0)) {
	                        /* Search for key within section */
	                        for (e = h->entries; e != NULL; e = e->next) {
	                                if (mywstrncasecmp(key,e->key,0)) {
	                                        /* Found! */
	                                        return e->value;
	                                }
	                        }
	                }
	        }
	} else {
	        /* Search for section name */
	        for (h = cfg; h != NULL; h = h->next) {
	                if (mystrncasecmp(section, h->section, 0)) {
	                        /* Search for key within section */
	                        for (e = h->entries; e != NULL; e = e->next) {
	                                if (mystrncasecmp(key, e->key, 0)) {
	                                        /* Found! */
	                                        return e->value;
	                                }
	                        }
	                }
	        }
	}
        /* Key not found in section */
        return NULL;
}

/* Return last value in specified section */
CFG_Entry *CFG_FindLastSectionEntry(CFG_Header *file_info, unsigned char *section, bool Unicode)
{
	CFG_Header 	*h;
	CFG_Entry	*e;

	e = NULL;
	/* First find our section */
        for (h = file_info; h != NULL; h = h->next) {
		if (Unicode) {
			if (mywstrncasecmp(section, h->section, 0)) {
				e = h->entries;
				break;
			}
		} else {
			if (mystrncasecmp(section, h->section, 0)) {
				e = h->entries;
				break;
			}
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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
