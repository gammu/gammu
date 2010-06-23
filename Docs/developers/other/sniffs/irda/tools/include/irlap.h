/*********************************************************************
 *
 * Filename:      irlap.h
 * Version:
 * Description:   in this file you can only find define큦 for parts
 *                of the irlap-layer. If you want read the typedef큦
 *                for struct큦 look into irda.h
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
/*
 * in this file you can only find define큦 for parts
 * of the irlap-layer. If you want read the typedef큦
 * for struct큦 look into irda.h
 */

#ifndef __IRLAP_H
#define __IRLAP_H

#include <glib.h>


#define IRDA_XBOF 0xFF
#define IRDA_BOF  0xC0
#define IRDA_EOF  0xC1

/*
 * IrLAP-OFFSETS
 * -------------
 */
#define IRLAP_A_OFF 0x00
#define IRLAP_C_OFF 0x01
#define IRLAP_I_OFF 0x02

/*
 * IrLAP-DEFINES
 * -------------
 */
#define IRLAP_XBOF 0xFF
#define IRLAP_BOF  0xC0
#define IRLAP_EOF  0xC1
#define IRLAP_CE   0x7D
#define IRLAP_ESC  IRLAP_CE

#define IRLAP_ESC_MASK (~0xDF)

#define IRLAP_A_LENGTH   1
#define IRLAP_C_LENGTH   1
#define IRLAP_FCS_LENGTH 2
#define IRLAP_EOF_LENGTH 1

#define IRLAP_C_MASK            0x03
#define IRLAP_U_FRAME           0x03
#define IRLAP_S_FRAME           0x01
#define IRLAP_PF_BIT_MASK     (~0xEF)
#define IRLAP_PF_BIT_CLR_MASK (~0x10)
#define IRLAP_Nr_CLR_MASK       0x1F

/*
 * IRLAP U cmd/response
 * --------------------
 * - all defines use a cleared P/F-Bit
 *   (irlap_frame -> c & IRLAP_PF_BIT_CLR_MASK)
 * - page 26 in IrLAP-spec
 * - if no _CMD or _RSP name is _CMD_RSP (e.g. 0x83)
 */
#define IRLAP_SNRM_RNRM 0x83
#define IRLAP_DISC_RD   0x43
#define IRLAP_UI_UI     0x03
#define IRLAP_XID_CMD   0x2F
#define IRLAP_TEST_TEST 0xE3
#define IRLAP_UA_RSP    0x63
#define IRLAP_FRMR_RSP  0x87
#define IRLAP_DM_RSP    0x0F
#define IRLAP_XID_RSP   0xAF

/*
 * IRLAP S cmd/response
 * --------------------
 * - all defines use a cleared P/F-Bit and cleared Nr
 *   (irlap_frame -> c & IRLAP_PF_BIT_CLR_MASK & IRLAP_Nr_CLR_MASK)
 * - page 32 in IrLAP-spec
 * - for both (cmd and response) the same mask is defined
 */
#define IRLAP_RR   0x01
#define IRLAP_RNR  0x05
#define IRLAP_REJ  0x09
#define IRLAP_SREJ 0x0D


/*
 * irlap-connection-states
 * -----------------------
 * - negotation defines are also conn-states
 */
#define IRLAP_DISC     0x00		/* disconnected          */
#define IRLAP_NDM      0x01		/* normaldisconnect mode */
#define IRLAP_NRM      0x02		/* normal response mode  */

/*
 * negotation-defines
 * ------------------
 */
/* negotation-states */
#define NEG_SNRM_OK     0x01
#define NEG_WAIT_FOR_UA 0x02
#define NEG_UA_OK       0x03
#define NEG_COMPLETE    0x04

#define B576000   576000
#define B1152000 1152000

#define IRLAP_SNRM_SRC_ADR       0x00 /* src in SNRM               */
#define IRLAP_SNRM_DEST_ADR      0x04 /* dest in SNRM              */
#define IRLAP_SNRM_CONN_ADR      0x08 /* connection-adr in SNRM    */
#define IRLAP_UA_SRC_ADR         0x00 /* src in UA                 */
#define IRLAP_UA_DEST_ADR        0x04 /* dest in UA                */
#define IRLAP_NEG_SNRM_PARAM_OFF 0x09 /* src(U32) dest(U32) adr(8) */
#define IRLAP_NEG_UA_PARAM_OFF   0x08 /* src(U32) dest(U32)        */
#define IRLAP_NEG_BPS_PI         0x01 /* IrLAP page 39             */
#define IRLAP_PI_LENGTH 1	/* no more longer used */
#define IRLAP_PL_LENGTH 1	/* no more longer used */

/*
 * defines for decode xid-cmd
 * --------------------------
 */
#define XID_C_FI_OFFSET    0x00	/* offset for format identifier (U8)  */
#define XID_C_SRC_ADR_OFF  0x01	/* offset for source address    (U32) */
#define XID_C_DEST_ADR_OFF 0x05	/* offset for dest address      (U32) */
#define XID_C_DISC_F_OFF   0x09	/* offset for discovery flags   (U8)  */
#define XID_C_SLOT_NR_OFF  0x0A	/* offset for slot number       (U8)  */
#define XID_C_VERSION_OFF  0x0B	/* offset for version           (U8)  */
#define XID_C_DISC_I_OFF   0x0C /* offset for discovery information   */
#define XID_C_FLAG_TRUE    0x01	/* flags in cmd only if FI == 0x01    */
#define XID_C_F_SLOT_MASK  0x03 /* mask for slot numbers in di-flag   */
#define XID_C_F_NEW_ADR_M  0x04 /* mask for new device-address-gen.   */
#define XID_C_LAST_SLOT    0xFF /* nr.  for last slot in xid-cmd      */
#define XID_C_DISC_I_M_L   0x20 /* max length of discovery info       */
#define XID_C_DI_S_H_OFF   0x0C /* 1. service hint offset in disc-inf */
#define XID_C_DI_S_H_EX_M  0x07 /* serv.hint extent. mask in disc_inf */

/*
 * defines for decode xid-rsp
 * --------------------------
 */
#define XID_R_FI_OFFSET    0x00	/* offset for format identifier (U8)  */
#define XID_R_SRC_ADR_OFF  0x01	/* offset for source address    (U32) */
#define XID_R_DEST_ADR_OFF 0x05	/* offset for dest address      (U32) */
#define XID_R_DISC_F_OFF   0x09	/* offset for discovery flags   (U8)  */
#define XID_R_SLOT_NR_OFF  0x0A	/* offset for slot number       (U8)  */
#define XID_R_VERSION_OFF  0x0B	/* offset for version           (U8)  */
#define XID_R_DISC_I_OFF   0x0C /* offset for discovery information   */
#define XID_R_FLAG_TRUE    0x01	/* flags in rsp only if FI == 0x01    */
#define XID_R_F_SLOT_MASK  0x03 /* mask for slot numbers in di-flag   */
#define XID_R_F_NEW_ADR_M  0x04 /* mask for new device-address-gen.   */
#define XID_R_LAST_SLOT    0xFF /* nr.  for last slot in xid-cmd/rsp  */
#define XID_R_DISC_I_M_L   0x20 /* max length of discovery info       */
#define XID_R_DI_S_H_OFF   0x0C /* 1. service hint offset in disc-inf */
#define XID_R_DI_S_H_EX_M  0x07 /* serv.hint extent. mask in disc_inf */
#define XID_R_SNIFF_FRAME  0xFFFFFFFF /* dest address for sniffing    */

/* 
 * defines for snrm-cmd
 * --------------------
 */
#define SNRM_CMD_RESET_LEN   0x00 /* no i-field is present              */
#define SNRM_EST_CONNECT_ADR 0xFF /* conn.adr. in case con.establish    */
#define SNRM_C_SRC_ADR_OFF   0x00 /* offset for source address    (U32) */
#define SNRM_C_DEST_ADR_OFF  0x04 /* offset for dest address      (U32) */
#define SNRM_C_N_C_ADR_OFF   0x08 /* new conn. adr. offset        (U8)  */
#define SNRM_C_NEG_OFF       0x09 /* negotation-fields offset           */
#define SNRM_BPS_PI          0x01 /* bps-pi                             */
#define SNRM_MAX_TAT_PI      0x82 /* max. turn around time pi           */
#define SNRM_DATA_SIZE_PI    0x83 /* data size pi                       */
#define SNRM_WIN_SIZE_PI     0x84 /* window size pi                     */
#define SNRM_ADD_BOF_PI      0x85 /* addition BOFs pi                   */
#define SNRM_MIN_TAT_PI      0x86 /* min. turn around time pi           */
#define SNRM_LD_TT_PI        0x08 /* link disc./threshold time pi       */

/*
 * defines for ua-rsp-frame
 * ------------------------
 */
#define UA_CONNECT_MIN_LENGTH 0x0E /* 14 are min length to use the frame
				    * for response to snrm used to
				    * establish a connection - look at
				    * page 39 - negotation: type 0 params
				    * must be negotiated at both stations
				    * bps and link disc/threshold time -
				    * in this case also src and dest are
				    * used ( 4 + 4 + 3 + 3), page 31      */
#define UA_WITH_SRC_DST_LEN   0x08 /* ua len with src and dest            */
#define UA_SRC_ADR_OFF        0x00 /* offset for source address    (U32) */
#define UA_DEST_ADR_OFF       0x04 /* offset for dest address      (U32) */
#define UA_NEG_OFF            0x08 /* negotation-fields offset           */
#define UA_BPS_PI             0x01 /* bps-pi                             */
#define UA_MAX_TAT_PI         0x82 /* max. turn around time pi           */
#define UA_DATA_SIZE_PI       0x83 /* data size pi                       */
#define UA_WIN_SIZE_PI        0x84 /* window size pi                     */
#define UA_ADD_BOF_PI         0x85 /* addition BOFs pi                   */
#define UA_MIN_TAT_PI         0x86 /* min. turn around time pi           */
#define UA_LD_TT_PI           0x08 /* link disc./threshold time pi       */


#endif /* __IRLAP_H */

