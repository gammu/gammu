/*

  $Id: rlp-crc24.h,v 1.1 2001/02/21 19:57:11 chris Exp $

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for CRC24 (aka FCS) implementation in RLP.

*/

#ifndef __data_rlp_crc24_h
#define __data_rlp_crc24_h

#ifndef __misc_h
  #include    "misc.h"
#endif

/* Prototypes for functions */

void RLP_CalculateCRC24Checksum(u8 *data, int length, u8 *crc);
bool RLP_CheckCRC24FCS(u8 *data, int length);

#endif
