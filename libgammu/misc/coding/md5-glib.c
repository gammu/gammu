/**
 * Wrapper for glib checksum functionality.
 */
#include "md5.h"
#include <glib.h>
#include <string.h>

void CalculateMD5(unsigned char *buffer, int length, char *checksum)
{
    gchar *ret;
    ret = g_compute_checksum_for_data(G_CHECKSUM_MD5, buffer, length);
    strcpy(checksum, ret);
    g_free(ret);
}
