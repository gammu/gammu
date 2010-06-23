/*********************************************************************
 *
 * Filename:      irda.h
 * Version:
 * Description:   irda-specific defines
 * Status:        Experimental.
 * Author:        Thomas Schneider <nok-trace-men@dev-thomynet.de>
 * Created at:    
 * Modified at:   
 * Modified by:   Thomas Schneider <nok-trace-men@dev-thomynet.de>
 *
 *     Copyright (c) 1999 Thomas Schneider, All Rights Reserved.
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of
 *     the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * IN NO EVENT SHALL THOMAS SCHNEIDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES 
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THOMAS SCHNEIDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH 
 * DAMAGE.
 *
 * THOMAS SCHNEIDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER
 * IS ON AN "AS IS" BASIS, AND THOMAS SCHNEIDER HAS NO OBLIGATION TO 
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR 
 * MODIFICATIONS.
 *
 *         This material is provided "AS-IS" and at no charge.
 *
 ********************************************************************/

#ifndef __NOKTRACE_IRDA_H
#define __NOKTRACE_IRDA_H

#include <glib.h>


/*
 * service hints
 * -------------
 * - first byte
 */
#define PNP_COMP 0x00
#define PDA_PALM 0x01
#define COMPUTER 0x02
#define PRINTER  0x03
#define MODEM    0x04
#define FAX      0x05
#define LAN      0x06
#define EXTENS   0x07


/*
 * output-defines
 * --------------
 */
#define OUT_ALL        0x01
#define OUT_IRLAP      0x02
#define OUT_IRLMP      0x03
#define OUT_HIGH_LEVEL 0x04

#define OUT_FORM_LINE  0x01
#define OUT_FORM_STEP  0x02

#define OUT_SCREEN     0x01
#define OUT_FILE       0x02

/*
 * some types
 */
typedef unsigned char  BYTE;
typedef unsigned char  U8  ;
typedef unsigned short U16 ;
typedef unsigned int   U32 ;

/*
 * some structs
 */
typedef struct 
{
  U16  length;
  U16  a_offset;
  BYTE buf[4096];
} RAW_FRAME;

typedef struct 
{
  U16  bof_length;
  BYTE * bof;
  BYTE a;
  BYTE c;
  U16  info_length;
  BYTE * info;
  U32  fcs;			/* must be U32 because escaped fcs */
  BYTE eof_length;
  BYTE * eof;
  BYTE esc_nr;			/* all ce's - include also esc_fcs */
  BYTE esc_fcs;			/* nr of ce's in fcs               */
  U16  computed_fcs;
} IRLAP_FRAME;

typedef struct 
{
  U16  bof_length;
  BYTE * bof;
  BYTE a;
  BYTE c;
  U16  info_length;
  BYTE * info;
  U32  fcs;			/* must be U32 because escaped fcs */
  BYTE eof_length;
  BYTE * eof;
  BYTE esc_nr;			/* all ce's - include also esc_fcs */
  BYTE esc_fcs;			/* nr of ce's in fcs               */
  U16  computed_fcs;
} IRLAP_RAW_FRAME;

typedef struct 
{
  U8    cmd;
  U8    dlsap_sel;
  U8    slsap_sel;
  U16   info_length;
  U8  * info;
} IRLMP_FRAME;

typedef struct 
{
  BYTE baud_rate_master;
  BYTE baud_rate_client;
  BYTE baut_rate_connection;
  BYTE state;
} NEGOTATION_PARAM;

typedef struct
{
  BYTE irlap_state;
  BYTE irlmp_state;
  U32  p_src_adr;		/* primary - source address   */
  U32  s_src_adr;		/* secondary - source address */
  U8   conn_adr;		/* connection address         */
} CONNECTION;

typedef struct
{
  U32  prim_nr_bytes;		/* bytes from primary station   */
  U32  sec_nr_bytes ;		/* bytes from secomdary station */
  U32  prim_fcs_errors;		/* nr of fcs-errors from prim.  */
  U32  sec_fcs_errors;		/* nr of fcs-errors from sec.   */
  U32  prim_frames;		/* frames from primary          */
  U32  sec_frames;		/* frames from secondary        */
  U32  prim_esc_nr;		/* nr of ecsapes from primary   */
  U32  sec_esc_nr;		/* nr of escapes from secondary */
} CONNECT_STATISTIC;

typedef struct
{
  BYTE deep;			/* deep of output (L2, L3, ...  */
  BYTE form;			/* kind of output (line, steps) */
  BYTE bit_output;		/* if output as bits            */
  BYTE dest;			/* file, screen                 */
  FILE input_fd;		/* input FILE                   */
  GString infile_name;		/* name of input-file           */
  FILE output_file;		/* output FILE                  */
  GString outfile_name;		/* name of input-file           */
} OUTPUT_PARAMS;


#endif /* __NOKTRACE_IRDA_H */

