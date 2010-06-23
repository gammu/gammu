/*********************************************************************
 *                
 * Filename:      fcs.h
 * Version:       
 * Description:   fcs routines for irda
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Mon Aug  4 20:40:53 1997
 * Modified at:   Sat Jul 17 08:43:59 1999
 * Modified by:   Thomas Schneider <nok-trace-men@dev-thomynet.de>
 * Sources:       crc.h by Dag Brattli <dagb@cs.uit.no> in
 *                linux-irda-project
 * 
 ********************************************************************/

#ifndef FCS_H
#define FCS_H

#include <linux/types.h>

#define INIT_FCS  0xffff   /* Initial FCS value    */
#define GOOD_FCS  0xf0b8   /* Good final FCS value */

#define IR_FCS(fcs, byte)((fcs >> 8)^irda_fcs16_table[ (fcs^byte) & 0xff])

unsigned short fcs_calc( __u16 oldfcs, __u8 *buf, int len);

extern __u16 irda_fcs16_table[];

#endif
