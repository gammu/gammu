/* (c) 2002-2004 by Marcin Wiacek */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifndef __OpenBSD__
#  include <wchar.h>
#  include <wctype.h>
#endif

#include "../config.h"
#include "coding/coding.h"
#include "cfg.h"
#include "misc.h"

/* 
 * Read information from file in Windows INI format style
 */
INI_Section *INI_ReadFile(char *FileName, bool Unicode)
{
	FILE		*f;
	bool		FFEEUnicode=false;
	int		level = -1, buffer1used, buffer2used;
	int		bufferused, i, buffused=1000,buffread=1000, num;
	unsigned char	ch[3], *buffer = NULL;
	unsigned char	*buffer2 = NULL, *buffer1 = NULL, buff[1000];
        INI_Section 	*INI_info = NULL, *INI_head = NULL, *heading;
        INI_Entry 	*entry;

	f = fopen(FileName,"rb");
	if (f == NULL) return NULL;
	
	num = 0;
	while(1) {
		/* We read one line from file */
		bufferused = 0;
		while (1) {
			if (buffused == buffread) {
				buffused = fread(buff,1,1000,f);
				buffread = 0;
				if (buffused == 0) {
					free(buffer); free(buffer1); free(buffer2);
					fclose(f);
					return INI_head;
				}
			}
			if (Unicode) {
				if (num == 0) {
					if (buffused == buffread) continue;
					ch[0] = buff[buffread++];
					num = 1;
				}
				if (num == 1) {
					if (buffused == buffread) continue;
					ch[1] = buff[buffread++];
					num = 0;				
				}
				if (level == -1) {
					if (ch[0] == 0xFF && ch[1] == 0xFE) FFEEUnicode = true;
					level = 0;
					continue;
				}
				if (FFEEUnicode) {
					ch[2] = ch[0]; ch[0] = ch[1]; ch[1] = ch[2];
				}
			} else {
				if (buffused == buffread) continue;
				ch[0] = 0;
				ch[1] = buff[buffread++];
				if (level == -1) level = 0;
			}
			if ((ch[0] == 0 && ch[1] == 13) ||
			    (ch[0] == 0 && ch[1] == 10)) {
				break;
			}
			buffer 			= realloc(buffer,bufferused+2);
			buffer[bufferused] 	= ch[0];
			buffer[bufferused+1] 	= ch[1];
			bufferused		= bufferused + 2;
		}
//		printf("line \"%s\"\n",DecodeUnicodeConsole(buffer));

		buffer1used = 0;
		buffer2used = 0;
		if (level == 1) level = 0;
		if (level == 3 || level == 4 || level == 5) level = 2;

		/* We parse read line */
		for (i=0;i<bufferused/2;i++) {
			ch[0] = buffer[i*2];
			ch[1] = buffer[i*2+1];
			if (level == 0) { //search for name of section
				if (ch[0] == 0 && ch[1] == '[') level = 1;
				if (ch[0] == 0 && ch[1] == ';') break;
				if (ch[0] == 0 && ch[1] == '#') break;
				continue;
			}
			if (level == 1) { //section name
				if (ch[0] == 0 && ch[1] == ']') {
					if (buffer1used == 0) break;
					if (Unicode) {
						buffer1 		= realloc(buffer1,buffer1used+2);
						buffer1[buffer1used] 	= 0;
						buffer1[buffer1used+1] 	= 0;
						buffer1used		= buffer1used + 2;
					} else {
						buffer1 		= realloc(buffer1,buffer1used+1);
						buffer1[buffer1used] 	= 0x00;
						buffer1used		= buffer1used + 1;
					}
					heading = (INI_Section *)malloc(sizeof(*heading));
		                        if (heading == NULL) {
						free(buffer); free(buffer1); free(buffer2);
						fclose(f);
		                                return NULL;
		                        }
					heading->SectionName = (char *)malloc(buffer1used);
					memcpy(heading->SectionName,buffer1,buffer1used);
		                        heading->Prev = INI_info;
					heading->Next = NULL;
		                        if (INI_info != NULL) {
		                                INI_info->Next  = heading;
		                        } else {
		                                INI_head 	= heading;
		                        }
		                        INI_info 		= heading;
					INI_info->SubEntries 	= NULL;
					level 	 		= 2;
//					printf("[%s]\n",DecodeUnicodeConsole(buffer1));
					break;
				}
				if (Unicode) {
					buffer1 		= realloc(buffer1,buffer1used+2);
					buffer1[buffer1used] 	= ch[0];
					buffer1[buffer1used+1] 	= ch[1];
					buffer1used		= buffer1used + 2;
				} else {
					buffer1 		= realloc(buffer1,buffer1used+1);
					buffer1[buffer1used] 	= ch[1];
					buffer1used		= buffer1used + 1;
				}
				continue;
			}
			if (level == 2) { //search for key name
				if (ch[0] == 0 && ch[1] == ';') break;
				if (ch[0] == 0 && ch[1] == '#') break;
				if (ch[0] == 0 && ch[1] == '[') {
					level = 1;
					continue;
				}
				if (Unicode) {
			                if (myiswspace(ch)) continue;
				} else {
			                if (isspace((int) ch[1])) continue;
				}
				level = 3;
			}
			if (level == 3) { //key name
				if (ch[0] == 0 && ch[1] == '=') {
					if (buffer1used == 0) break;
					while(1) {
						if (Unicode) {
					                if (!myiswspace(buffer1+(buffer1used-2))) break;
							buffer1used = buffer1used - 2;
						} else {
					                if (!isspace((int)buffer1[buffer1used-1])) break;
							buffer1used = buffer1used - 1;	
						}
					}
					level = 4;
					continue;
				}				
				if (Unicode) {
					buffer1 		= realloc(buffer1,buffer1used+2);
					buffer1[buffer1used] 	= ch[0];
					buffer1[buffer1used+1] 	= ch[1];
					buffer1used		= buffer1used + 2;
				} else {
					buffer1 		= realloc(buffer1,buffer1used+1);
					buffer1[buffer1used] 	= ch[1];
					buffer1used		= buffer1used + 1;
				}
			}
			if (level == 4) { //search for key value
				if (Unicode) {
			                if (myiswspace(ch)) continue;
				} else {
			                if (isspace((int) ch[1])) continue;
				}
				level = 5;
			}
			if (level == 5) { //key value
				if (Unicode) {
					buffer2 		= realloc(buffer2,buffer2used+2);
					buffer2[buffer2used] 	= ch[0];
					buffer2[buffer2used+1] 	= ch[1];
					buffer2used		= buffer2used + 2;
				} else {
					buffer2 		= realloc(buffer2,buffer2used+1);
					buffer2[buffer2used] 	= ch[1];
					buffer2used		= buffer2used + 1;
				}
			}
		}
		if (level == 5) {
			if (buffer2used == 0) continue;

			entry = (INI_Entry *)malloc(sizeof(*entry));
                        if (entry == NULL) {
				free(buffer); free(buffer1); free(buffer2);
				fclose(f);
                                return NULL;
                        }
			if (Unicode) {
				buffer1 		= realloc(buffer1,buffer1used+2);
				buffer1[buffer1used] 	= 0;
				buffer1[buffer1used+1] 	= 0;
				buffer1used		= buffer1used + 2;
				buffer2 		= realloc(buffer2,buffer2used+2);
				buffer2[buffer2used] 	= 0;
				buffer2[buffer2used+1] 	= 0;
				buffer2used		= buffer2used + 2;
			} else {
				buffer1 		= realloc(buffer1,buffer1used+1);
				buffer1[buffer1used] 	= 0x00;
				buffer1used		= buffer1used + 1;
				buffer2 		= realloc(buffer2,buffer2used+1);
				buffer2[buffer2used] 	= 0x00;
				buffer2used		= buffer2used + 1;
			}
//			printf("\"%s\"=\"%s\"\n",buffer1,buffer2);
//			printf("\"%s\"=",DecodeUnicodeConsole(buffer1));
//			printf("\"%s\"\n",DecodeUnicodeConsole(buffer2));

			entry->EntryName = (char *)malloc(buffer1used);
			memcpy(entry->EntryName,buffer1,buffer1used);

			entry->EntryValue = (char *)malloc(buffer2used);
			memcpy(entry->EntryValue,buffer2,buffer2used);

			entry->Prev = NULL;
                        entry->Next = INI_info->SubEntries;
                        if (INI_info->SubEntries != NULL) INI_info->SubEntries->Prev = entry;
                        INI_info->SubEntries = entry;
		}
	}
	free(buffer); free(buffer1); free(buffer2);
	fclose(f);
	return INI_head;
}

/* 
 * Search for key value in file in Windows INI format style
 * Returns found value or NULL
 */
unsigned char *INI_GetValue(INI_Section *cfg, unsigned char *section, unsigned char *key, bool Unicode)
{
        INI_Section 	*sec;
        INI_Entry  	*ent;

        if (cfg == NULL || section == NULL || key == NULL) return NULL;

	if (Unicode) {
	        /* Search for section */
		sec = cfg;
		while (sec != NULL) {
	                if (mywstrncasecmp(section, sec->SectionName, 0)) {
	                        /* Search for key inside section */
				ent = sec->SubEntries;
				while (ent != NULL) {
	                                if (mywstrncasecmp(key,ent->EntryName,0)) {
	                                        return ent->EntryValue;
	                                }
					ent = ent->Next;
	                        }
	                }
			sec = sec->Next;
		}
	} else {
	        /* Search for section */
		sec = cfg;
		while (sec != NULL) {
	                if (mystrncasecmp(section, sec->SectionName, 0)) {
	                        /* Search for key inside section */
				ent = sec->SubEntries;
				while (ent != NULL) {
	                                if (mystrncasecmp(key,ent->EntryName,0)) {
	                                        return ent->EntryValue;
	                                }
					ent = ent->Next;
	                        }
	                }
			sec = sec->Next;
		}
	}
        return NULL;
}

/* Return last value in specified section */
INI_Entry *INI_FindLastSectionEntry(INI_Section *file_info, unsigned char *section, bool Unicode)
{
	INI_Section 	*h;
	INI_Entry	*e;

	e = NULL;
	/* First find our section */
        for (h = file_info; h != NULL; h = h->Next) {
		if (Unicode) {
			if (mywstrncasecmp(section, h->SectionName, 0)) {
				e = h->SubEntries;
				break;
			}
		} else {
			if (mystrncasecmp(section, h->SectionName, 0)) {
				e = h->SubEntries;
				break;
			}
		}
	}

	if (e == NULL) return NULL;
	
	/* Goes into last value in section */
	while (e->Next != NULL) e = e->Next;
	return e;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
