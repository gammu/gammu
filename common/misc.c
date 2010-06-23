/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef WIN32
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <stdlib.h>
  #include <fcntl.h>
  #include <signal.h>
  #include <unistd.h>
  #include <errno.h>
#endif

#include "misc.h"
#include "gsm-common.h"

#ifndef HAVE_TIMEOPS

/* FIXME: I have timersub defined in sys/time.h :-( PJ
   FIXME: Jano wants this function too... PJ

int timersub(struct timeval *a, struct timeval *b, struct timeval *result) {
  do {
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;
    if ((result)->tv_usec < 0) {
      --(result)->tv_sec;
      (result)->tv_usec += 1000000;
    }
  } while (0);
}
*/

#endif

int GetLine(FILE *File, char *Line, int count) {

  char *ptr;

  if (fgets(Line, count, File)) {
    ptr=Line+strlen(Line)-1;

    while ( (*ptr == '\n' || *ptr == '\r') && ptr>=Line) *ptr--='\0';

    return strlen(Line);
  } else return -1;
}

/*
 * like atoi, but of a non-null-terminated string of a specified portion
 */
int mem_to_int(const char str[], int len)
{
  char aux[81];

  strncpy(aux, str, len);
  aux[len]=0;
  return( atoi(aux) );
} 

/*
 * make hexdump of Message
 */
#ifdef DEBUG
void hexdump(u16 MessageLength, u8 *MessageBuffer)
{
 
  int count;
  int n=0;
  char string1[80]="";
  char string2[80]="";
  char hex1[10];
  char hex2[10];
 
  for (count = 0; count < MessageLength; count ++)
  {
    n++;

    switch (MessageBuffer[count]) {
      case 0x09:
        sprintf(hex1,"%02x  ",MessageBuffer[count]);
        strcpy(hex2,".");
        break;
      default:
        if (isprint(MessageBuffer[count]))
          sprintf(hex1,"%02x%c ",MessageBuffer[count],MessageBuffer[count]);
        else
          sprintf(hex1,"%02x  ",MessageBuffer[count]);

        if (isprint(MessageBuffer[count])) sprintf(hex2,"%c",MessageBuffer[count]);
                                      else strcpy(hex2,".");
        break;
    }

    if ( n!=15 && count != MessageLength-1 ) hex1[3]='|';
 
    strcat(string1,hex1);
    strcat(string2,hex2);
 
    if ( n==15 || count == MessageLength-1 )
    {      
      fprintf(stdout,"%-60s%03x %s\n",string1,count+1,string2);
      strcpy(string1,"");
      strcpy(string2,"");
      n=0;
    }
  }//for count

  if (n!=0) fprintf (stdout,_("\n")); 
 
  fflush(stdout);
}

void txhexdump(u16 MessageLength, u8 *MessageBuffer)
{ 
  int count;
  int n=0;
 
  for (count = 0; count < MessageLength; count ++)
   {
    n++;
    fprintf(stdout,_("%02x"),MessageBuffer[count]);
    switch (MessageBuffer[count]) {
      case 0x09:
        fprintf(stdout,_(" |"));
        break;
      default:
        if (isprint(MessageBuffer[count])) fprintf(stdout, _("%c|"),MessageBuffer[count]);
                                      else fprintf(stdout,_(" |"));
        break;
    }

    if (n==18)
    { 
      fprintf (stdout,_("\n"));
      n=0;
    }
   }//for count

  if (n!=0) fprintf (stdout,_("\n")); 

  fflush(stdout);
}
#endif
