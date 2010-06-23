/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "xgnokii_cfg.h"
#include "xgnokii.h"

ConfigEntry config[] = {
 {"name",      &(xgnokiiConfig.user.name)},
 {"title",     &(xgnokiiConfig.user.title)},
 {"company",   &(xgnokiiConfig.user.company)},
 {"telephone", &(xgnokiiConfig.user.telephone)},
 {"fax",       &(xgnokiiConfig.user.fax)},
 {"email",     &(xgnokiiConfig.user.email)},
 {"address",   &(xgnokiiConfig.user.address)},
 {"viewer",    &(xgnokiiConfig.helpviewer)},
 {"mailbox",   &(xgnokiiConfig.mailbox)},
 {"simlen",    &(xgnokiiConfig.maxSIMLen)},
 {"phonelen",  &(xgnokiiConfig.maxPhoneLen)},
 {"",          NULL}
};


static void GetDefaultValues ()
{
  gchar *homedir;
  
  xgnokiiConfig.user.name = g_strdup ("");
  xgnokiiConfig.user.title = g_strdup ("");
  xgnokiiConfig.user.company = g_strdup ("");
  xgnokiiConfig.user.telephone = g_strdup ("");
  xgnokiiConfig.user.fax = g_strdup ("");
  xgnokiiConfig.user.email = g_strdup ("");
  xgnokiiConfig.user.address = g_strdup ("");
  xgnokiiConfig.helpviewer = g_strdup ("netscape");
  if ((homedir = g_get_home_dir ()) == NULL)
    homedir = "";
  xgnokiiConfig.mailbox = g_strdup_printf ("%s/Mail/smsbox", homedir);
  xgnokiiConfig.maxSIMLen = g_strdup ("14");
  xgnokiiConfig.maxPhoneLen = g_strdup ("16");
}


void GUI_ReadXConfig ()
{
  FILE *file;
  gchar *line;
  gchar *homedir;
  gchar *rcfile;
  gchar *current;
  register gint len;
  register gint i;

  GetDefaultValues ();

#ifdef WIN32
/*  homedir = getenv("HOMEDRIVE");
  g_strconcat(homedir, getenv("HOMEPATH"), NULL); */
  homedir = g_get_home_dir ();
  rcfile=g_strconcat(homedir, "\\_xgnokiirc", NULL);
#else
  if ((homedir = g_get_home_dir ()) == NULL)
  {
    g_print (_("WARNING: Can't find HOME enviroment variable!\n"));
    return;
  }

  if ((rcfile = g_strconcat (homedir, "/.xgnokiirc", NULL)) == NULL)
  {
    g_print (_("WARNING: Can't allocate memory for config reading!\n"));
    return;
  }
#endif

  if ((file = fopen (rcfile, "r")) == NULL)
  {
    g_free (rcfile);
    return;
  }

  g_free (rcfile);

  if ((line = (char *) g_malloc (255)) == NULL)
  {
    g_print (_("WARNING: Can't allocate memory for config reading!\n"));
    fclose (file);
    return;
  }

  while (fgets (line, 255, file) != NULL)
  {
    gint v;
    current = line;

    /* Strip leading, trailing whitespace */
    while (isspace ((gint) *current))
      current++;

    while ((strlen (current) > 0) && isspace ((gint) current[strlen (current) - 1]))
      current[strlen (current) - 1] = '\0';

    /* Ignore blank lines and comments */

    if ((*current == '\n') || (*current == '\0') || (*current == '#'))
      continue;

    i = 0;
    while (*config[i].key != '\0')
    {
      len = strlen (config[i].key);
      if (g_strncasecmp (config[i].key, current, len) == 0)
      {
        current += len;
        while (isspace ((int) *current))
          current++;
        if (*current == '=')
        {
          current++;
          while(isspace ((int) *current))
            current++;
          g_free (*config[i].value);
          switch (i)
          {
            case 3:
            case 4: 
              *config[i].value = g_strndup (current, max_phonebook_number_length);
              break;

            case 7:
              *config[i].value = g_strndup (current, HTMLVIEWER_LENGTH);
              break;

            case 8:
              *config[i].value = g_strndup (current, MAILBOX_LENGTH);
              break;

            case 9:
            case 10:
              v = atoi (current);
              if ( v > 0 && v < 100 )
                *config[i].value = g_strndup (current, 3);
              break;

            default:
              *config[i].value = g_strndup (current, MAX_BUSINESS_CARD_LENGTH);
              break;
          }
        }
      }
      i++;
    }
  }

  fclose (file);
  g_free (line);
}


gint GUI_SaveXConfig ()
{
  FILE *file;
  gchar *line;
  gchar *homedir;
  gchar *rcfile;
  register gint i;

  if ((homedir = getenv ("HOME")) == NULL)
  {
    g_print (_("ERROR: Can't find HOME enviroment variable!\n"));
    return (1);
  }

  if ((rcfile = g_strconcat (homedir, "/.xgnokiirc", NULL)) == NULL)
  {
    g_print (_("ERROR: Can't allocate memory for config writing!\n"));
    return (2);
  }

  if ((file = fopen (rcfile, "w")) == NULL)
  {
    g_print (_("ERROR: Can't open file %s for writing!\n"), rcfile);
    g_free (rcfile);
    return (3);
  }

  g_free (rcfile);

  i = 0;
  while (*config[i].key != '\0')
  {
    if ((line = g_strdup_printf ("%s = %s\n", config[i].key, *config[i].value)) == NULL)
    {
      g_print (_("ERROR: Can't allocate memory for config writing!\n"));
      fclose (file);
      return (2);
    }
    if (fputs (line, file) == EOF)
    {
      g_print (_("ERROR: Can't write config file!\n"));
      g_free (line);
      fclose (file);
      return (4);
    }
    g_free (line);
    i++;
  }

  fclose (file);
  return (0);
}
