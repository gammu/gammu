/*

  $Id: rlp-common.h,v 1.2 2001/06/10 11:29:49 machek Exp $

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  The development of RLP protocol is sponsored by SuSE CR, s.r.o. (Pavel use
  the SIM card from SuSE for testing purposes).

  Header file for RLP protocol.

*/

#ifndef __data_rlp_common_h
#define __data_rlp_common_h

#ifndef __misc_h
  #include "misc.h"
#endif

/* Global variables */

/* Defines */

/* Data types */

/* Typedef for frame type - they are the same for RLP version 0, 1 and 2. */

typedef enum {
  RLPFT_X, /* Unknown. */
  RLPFT_U, /* Unnumbered frame. */
  RLPFT_S, /* Supervisory frame. */
  RLPFT_IS /* Information plus Supervisory (I+S) frame. */
} RLP_FrameType;

/* Define the various Unnumbered frame types. Numbering is bit reversed
   relative to ETSI GSM 04.22 for easy parsing. */

typedef enum {
  RLPU_SABM  = 0x07, /* Set Asynchronous Balanced Mode. */
  RLPU_UA    = 0x0c, /* Unnumbered Acknowledge. */
  RLPU_DISC  = 0x08, /* Disconnect. */
  RLPU_DM    = 0x03, /* Disconnected Mode. */
  RLPU_NULL  = 0x0f, /* Null information. */
  RLPU_UI    = 0x00, /* Unnumbered Information. */
  RLPU_XID   = 0x17, /* Exchange Identification. */
  RLPU_TEST  = 0x1c, /* Test. */
  RLPU_REMAP = 0x11  /* Remap. */
} RLP_UFrameType;

/* Define supervisory frame field. */

typedef enum {
  RLPS_RR   = 0x00, /* Receive Ready. */
  RLPS_REJ  = 0x02, /* Reject. */
  RLPS_RNR  = 0x01, /* Receive Not Ready. */
  RLPS_SREJ = 0x03  /* Selective Reject. */
} RLP_SFrameField;

/* Used for CurrentFrameType. */

typedef enum {
  RLPFT_U_SABM = 0x00,
  RLPFT_U_UA,
  RLPFT_U_DISC,
  RLPFT_U_DM,
  RLPFT_U_NULL,
  RLPFT_U_UI,
  RLPFT_U_XID,
  RLPFT_U_TEST,
  RLPFT_U_REMAP,
  RLPFT_S_RR,
  RLPFT_S_REJ,
  RLPFT_S_RNR,
  RLPFT_S_SREJ,
  RLPFT_SI_RR,
  RLPFT_SI_REJ,
  RLPFT_SI_RNR,
  RLPFT_SI_SREJ,
  RLPFT_BAD
} RLP_FrameTypes;

/* Frame definition for TCH/F9.6 frame. */

typedef struct {
  u8 Header[2];
  u8 Data[25];    
  u8 FCS[3];
} RLP_F96Frame;   

/* Header data "split up" for TCH/F9.6 frame. */

typedef struct {
  u8            Ns;   /* Send sequence number. */
  u8            Nr;   /* Receive sequence number. */
  u8            M;    /* Unumbered frame type. */
  u8            S;    /* Status. */
  bool          PF;   /* Poll/Final. */
  bool          CR;   /* Command/Response. */
  RLP_FrameType Type; /* Frame type. */
} RLP_F96Header;


/* RLP User requests */

typedef struct {
  bool Conn_Req;
  bool Attach_Req;
  bool Conn_Req_Neg;
  bool Reset_Resp;
  bool Disc_Req;
} RLP_UserRequestStore;

typedef enum {
  Conn_Req,
  Attach_Req,
  Conn_Req_Neg,
  Reset_Resp,
  Disc_Req
} RLP_UserRequests;

typedef enum {
  Conn_Ind,
  Conn_Conf,
  Disc_Ind,
  Reset_Ind,
  Data,		/* FIXME: This should really be called RLP_Data, otherwise it hogs name "Data"! */
  StatusChange,
  GetData
} RLP_UserInds;

/* RLP (main) states. See GSM specification 04.22 Annex A, Section A.1.1. */

typedef enum {
  RLP_S0, /* ADM and Detached */
  RLP_S1, /* ADM and Attached */
  RLP_S2, /* Pending Connect Request */
  RLP_S3, /* Pending Connect Indication */
  RLP_S4, /* ABM and Connection Established */
  RLP_S5, /* Disconnect Initiated */
  RLP_S6, /* Pending Reset Request */
  RLP_S7, /* Pending Reset Indication */
  RLP_S8  /* Error */
} RLP_State;

/* RLP specification defines several states in which variables can be. */

typedef enum {
  _idle=0,
  _send,
  _wait,
  _rcvd,
  _ackn,
  _rej,
  _srej
} RLP_StateVariable;


/* RLP Data */

typedef struct { 
  u8 Data[25];
  RLP_StateVariable State;
} RLP_Data;



/* Prototypes for functions. */

void RLP_DisplayF96Frame(RLP_F96Frame *frame);
void RLP_DecodeF96Header(RLP_F96Frame *frame, RLP_F96Header *header);
void RLP_DisplayXID(u8 *frame);
void RLP_Initialise(bool (*rlp_send_function)(RLP_F96Frame *frame, bool out_dtx), int (*rlp_passup)(RLP_UserInds ind, u8 *buffer, int length));
void RLP_Init_link_vars(void);
void RLP_SetUserRequest(RLP_UserRequests type, bool value);
void RLP_Send(char *buffer, int length);

#endif	/* __data_rlp_common_h */
