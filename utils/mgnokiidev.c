/*

  $Id: mgnokiidev.c,v 1.6 2000/12/27 10:54:15 pkot Exp $
  
  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Mgnokiidev gets passed a slave pty name by gnokiid and uses this
  information to create a symlink from the pty to /dev/gnokii.

*/

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>

#define DEVLEN	30
#define MAXLEN	12

int main(int argc, char *argv[])
{
  int count, err;
  char dev_name[DEVLEN];

  /* Check we have one and only one command line argument. */
  if (argc != 2) {
    fprintf(stderr, "mgnokiidev takes one and only one argument!\n");
    exit(-2);
  }

  /* Check if argument has a reasonable length (less than MAXLEN characters) */
  if (strlen(argv[1]) >= MAXLEN) {
    fprintf(stderr, "Argument must be less than %d characters.\n", MAXLEN);
    exit (-2);
  }

  strncpy(dev_name, argv[1], DEVLEN);

  /* Check for suspicious characters. */
  for (count = 0; count < strlen(dev_name); count ++)
    if (!(isalnum(dev_name[count]) || dev_name[count]=='/')) {
      fprintf(stderr, "Suspicious character at index %d in argument.\n", count);
      exit (-2);
    }

  /* Now become root */
  setuid(0);

  /* Change group of slave pty to group of mgnokiidev */
  err = chown(dev_name, -1, getgid());

  if (err < 0) {
    perror("mgnokiidev - chown: ");
    exit (-2);
  }

  /* Change permissions to rw by group */
  err = chmod(dev_name, S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR);
	
  if (err < 0) {
    perror("mgnokiidev - chmod: ");
    exit (-2);
  }

  /* FIXME: Possible bug - should check that /dev/gnokii doesn't already exist
     in case multiple users are trying to run gnokii. Well, but will be
     mgnokiidev called then? I do not think so - you will probably got the
     message serialport in use or similar. Don't you. I haven't tested it
     though. */

  /* Remove symlink in case it already exists. Don't care if it fails.  */
  unlink ("/dev/gnokii");

  /* Create symlink */
  err = symlink(dev_name, "/dev/gnokii");

  if (err < 0) {
    perror("mgnokiidev - symlink: ");
    exit (-2);
  }

  /* Done */
  exit (0);
}
