/**
 * Wrapper for glib checksum functionality.
 */
#include "md5.h"
#include <glib.h>
#include <string.h>

void CalculateMD5(unsigned char *buffer, int length, char *checksum)
{
    strcpy(checksum, g_compute_checksum_for_data(G_CHECKSUM_MD5, buffer, length));
}
