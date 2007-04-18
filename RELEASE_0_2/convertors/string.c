/* Unicode strings conversion between Gammu and Python */
#include "convertors.h"

unsigned char *strPythonToGammu(const Py_UNICODE *src) {
    unsigned char *dest;
    int len = 0, i = 0;

    /* Get string length */
    while (src[len] != 0) len++;

    /* Allocate memory */
    dest = malloc((len + 1) * 2 * sizeof(char));
    if (dest == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }
    
    /* Go here up to len, to copy also last 0 */
    while (i <= len) {
        dest[i*2]       = (src[i] >> 8) & 0xff;
        dest[(i*2)+1]   = src[i] & 0xff;
        i++;
    }

    return dest;
}

Py_UNICODE *strGammuToPython(const unsigned char *src) {
    Py_UNICODE *dest;
    int len = 0, i = 0;

    /* Get string length */
    while (src[len*2] != 0 || src[(len*2)+1] != 0 ) len++;

    /* Allocate memory */
    dest = malloc((len + 3) * sizeof(Py_UNICODE));
    if (dest == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    /* Byte Order Mark */
    dest[0] = 0xfe;
    dest[1] = 0xff;

    /* Go here up to len, to copy also last 0 */
    while (i <= len) {
        dest[i] = (src[2*i] << 8) + src[(2*i) + 1];
        i++;
    }

    return dest;
}


