//
// PNGDIB.H
//

#ifndef _PNGDIB_H_
#define _PNGDIB_H_


// error codes returned by xOpenPNGFile() and/or xSavePNGFile()

#define PNGD_E_SUCCESS   0
#define PNGD_E_ERROR     1   // unspecified error 
#define PNGD_E_VERSION   2   // struct size problem
#define PNGD_E_NOMEM     3   // could not alloc memory
#define PNGD_E_UNSUPP    4   // unsupported image type
#define PNGD_E_LIBPNG    5   // libpng error (corrupt PNG?)
#define PNGD_E_BADBMP    6   // corrupt or unsupported DIB
#define PNGD_E_BADPNG    7   // corrupt or unsupported PNG
#define PNGD_E_READ      8   // couldn't read PNG file
#define PNGD_E_WRITE     9   // couldn't write PNG file


#ifdef __cplusplus
extern "C" {
#endif

int GetLastPNGError();
HANDLE xOpenPNGFile(const char *pFile, int *pDataOffset);
HANDLE xOpenPNGFile1(LPCTSTR pFile, int *pDataOffset, BOOL bAlpha);

#ifdef __cplusplus
}
#endif


#endif	// _PNGDIB_H_
