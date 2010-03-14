/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Functions for manipulating phonebook
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef WIN32
  #include "misc_win32.h"
#endif

#include "gsm-api.h"
#include "gsm-coding.h"

char *GSM_GetVCARD(GSM_PhonebookEntry *entry, int version) {

  static char Buffer[1000]="";

  u8 buffer[50*3];

  int len=0,i;

  bool pref=false;

  char memory_type_string[20];
  
  GetMemoryTypeString(memory_type_string, &entry->MemoryType);
                                        
  switch (version) {
    case 10:
      len+=sprintf(Buffer+len,_("BEGIN:VCARD%c%c"),13,10);
      len+=sprintf(Buffer+len,_("N:%s%c%c"),entry->Name,13,10);
      if (*entry->Number)
        len+=sprintf(Buffer+len,_("TEL:%s%c%c"),entry->Number,13,10);
      len+=sprintf(Buffer+len,_("END:VCARD%c%c"),13,10);
      break;
    case 21:
      len+=sprintf(Buffer+len,_("BEGIN:VCARD%c%cVERSION:2.1%c%c"),13,10,13,10);

      EncodeUTF8(buffer,entry->Name,strlen(entry->Name));
      if (strlen(entry->Name)==strlen(buffer)) {
	len+=sprintf(Buffer+len,_("N:%s%c%c"),entry->Name,13,10);
      } else {
	len+=sprintf(Buffer+len,_("N;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c"),buffer,13,10);
      }

      if (*entry->Number) {
        len+=sprintf(Buffer+len,_("TEL;PREF:%s%c%c"),entry->Number,13,10);
        pref=true;
      }
 		
      /* Rest of the subentries */
      for (i = 0; i < entry->SubEntriesCount; i++) {
        switch( entry->SubEntries[i].EntryType ) {
          case GSM_Number:
            switch( entry->SubEntries[i].NumberType ) {
              case GSM_General :len+=sprintf(Buffer+len,_("TEL"));break;
              case GSM_Home    :len+=sprintf(Buffer+len,_("TEL;HOME;VOICE"));break;
              case GSM_Mobile  :len+=sprintf(Buffer+len,_("TEL;CELL"));break;
              case GSM_Work    :len+=sprintf(Buffer+len,_("TEL;WORK;VOICE"));break;
              case GSM_Fax     :len+=sprintf(Buffer+len,_("TEL;FAX"));break;
	      default          :break;
            }
            if (!pref) {
              len+=sprintf(Buffer+len,_(";PREF"));
              pref=true;
            }
            break;
          case GSM_Note  :len+=sprintf(Buffer+len,_("NOTE"));break;
          case GSM_Postal:
            len+=sprintf(Buffer+len,_("LABEL"));
            EncodeUTF8(buffer,entry->SubEntries[i].data.Number,strlen(entry->SubEntries[i].data.Number));
            if (strlen(entry->SubEntries[i].data.Number)==strlen(buffer)) {
              len+=sprintf(Buffer+len,_(":%s%c%c"),entry->SubEntries[i].data.Number,13,10);
            } else {
              len+=sprintf(Buffer+len,_(";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c"),buffer,13,10);
            }
            len+=sprintf(Buffer+len,_("ADR"));
            break;
          case GSM_Email :len+=sprintf(Buffer+len,_("EMAIL"));break;
	  default        :break;
        }

        EncodeUTF8(buffer,entry->SubEntries[i].data.Number,strlen(entry->SubEntries[i].data.Number));
        if (strlen(entry->SubEntries[i].data.Number)==strlen(buffer)) {
          len+=sprintf(Buffer+len,_(":%s%c%c"),entry->SubEntries[i].data.Number,13,10);
        } else {
          len+=sprintf(Buffer+len,_(";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c"),buffer,13,10);
        }
      } 
      len+=sprintf(Buffer+len,_("END:VCARD%c%c"),13,10);
      break;
    case 30:
      len+=sprintf(Buffer+len,_("BEGIN:VCARD\nVERSION:3.0\n"));
      len+=sprintf(Buffer+len,_("FN:%s\n"),entry->Name);
      len+=sprintf(Buffer+len,_("TEL;PREF:%s\n"),entry->Number);
      len+=sprintf(Buffer+len,_("X_GSM_STORE_AT:%s%i\n"),memory_type_string,entry->Location);
      len+=sprintf(Buffer+len,_("X_GSM_CALLERGROUP:%i\n"),entry->Group);
      for( i = 0; i <  entry->SubEntriesCount; i++ )
      {
        if( entry->SubEntries[i].EntryType == GSM_Number ) {
  	    len+=sprintf(Buffer+len,_("TEL;UNKNOWN_%d:%s\n"),
	      entry->SubEntries[i].NumberType,
	      entry->SubEntries[i].data.Number );
	}
      }
      len+=sprintf(Buffer+len,_("END:VCARD\n\n"));
      break;
  }

  return Buffer;
}

int GSM_SavePhonebookEntryToSMS(GSM_MultiSMSMessage *SMS,
                                GSM_PhonebookEntry *entry, int version)
{
  char Buffer[1000]="";
  int length;
  GSM_UDH UDH=GSM_NoUDH;
  GSM_Coding_Type Coding=GSM_Coding_Default;

  sprintf(Buffer,GSM_GetVCARD(entry,version));  

  switch (version) {
    case 10:
      /* 1 SMS: no UDH */
      if (strlen(Buffer)<=160-8) { //8==length of ..SCKE2.
        sprintf(Buffer,"//SCKE2 ");  
        sprintf(Buffer,GSM_GetVCARD(entry,version));  
      } else {
        UDH=GSM_PhonebookUDH; //FIXME:is this true ?
//        Coding=GSM_Coding_8bit; //FIXME:is this true ?
      }
      break;
    case 21:
      /* 1 SMS: no UDH */
      if (strlen(Buffer)<=160-12) { //12==length of ..SCKL...
        sprintf(Buffer,_("//SCKL23F4%c%c"),13,10);  
        sprintf(Buffer,GSM_GetVCARD(entry,version));  
      } else {
        UDH=GSM_PhonebookUDH;
//        Coding=GSM_Coding_8bit; //can be 8 bit, but no abligatory
      }
      break;
  }

  length=strlen(Buffer);
  GSM_MakeMultiPartSMS2(SMS,Buffer,length, UDH, Coding);

  return 0;
}
