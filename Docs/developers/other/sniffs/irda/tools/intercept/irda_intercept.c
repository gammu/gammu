/*********************************************************************
 *
 * Filename:      irda_intercept.c
 * Version:
 * Description:   intercept irda-traffic incl. negotation and write
 *                output to a file
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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <glib.h>
#include "../include/irda.h"
#include "../include/irlap.h"
#include "../fcs/fcs.h"

#define INIT_BAUDRATE B9600

#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define INITIAL_TIMEOUT 15
#define DEFAULT_TIMEOUT  5

#define OUTFILE_SUFFIX ".trc"
#define DEFAULT_OUTFILE "out"OUTFILE_SUFFIX

static int initfdflags = -1;	     /* Initial file descriptor flags */
static struct termios old_port_sets; /* old port-termios for restore  */

int port_fd;

NEGOTATION_PARAM negotation_param;
CONNECTION connection;

static speed_t speed_list[8] = { B2400, B9600, B19200, B38400,
			     B57600, B115200, B576000, B1152000 };
static const char *speed_name[] = { "2400", "9600", "19200", "38400",
				"57600", "115200", "576000", "1152000"};

/*
 * negotation
 * ----------
 * - in negotation set new device speed
 */
void negotation ( IRLAP_FRAME * irlap_frame )
{
   BYTE cmd          = irlap_frame -> a & 0x01;
   speed_t new_speed = 0;
   int i             = 0;
   int m             = 0;
   struct termios set;
   BYTE tmp          = 0;

   /* 
    * snrm-frame
    * ----------
    * - 4 src
    * - 4 dest
    * - 1 new connection address
    * - 1 pi
    * - 1 pl
    * - [pl] pv
    * ua-frame
    * --------
    * - 4 src
    * - 4 dest
    * - 1 pi
    * - 1 pl
    * - [pl] pv
    */

   if ( cmd ) {
     /*
      * SNRM - CMD
      * ----------
      * - in IrLAP (V. 1.1) page 27
      * - U32 : src-dev-adr
      * - U32 : dest-dev-adr
      * - U8  : connection address
      * - start of neg. params
      */
     if ( irlap_frame -> info_length < ( IRLAP_NEG_SNRM_PARAM_OFF +3 ) ) {
       printf ("\t==> IrLAP: in SNRM-CMD no Info! <==\n");
     } else {
       memcpy ( &connection.p_src_adr,
		&(irlap_frame -> info[IRLAP_SNRM_SRC_ADR]),
		sizeof(connection.p_src_adr));
       memcpy ( &connection.s_src_adr,
		&(irlap_frame -> info[IRLAP_SNRM_DEST_ADR]),
		sizeof(connection.s_src_adr));
       connection.conn_adr = irlap_frame -> info[IRLAP_SNRM_CONN_ADR];
       if ( irlap_frame -> info[IRLAP_NEG_SNRM_PARAM_OFF] == 
	    IRLAP_NEG_BPS_PI ) {
	 /* really bps-parameter-identifier */
	 if ( irlap_frame -> info[IRLAP_NEG_SNRM_PARAM_OFF + 1] == 1 ) {
	   negotation_param.baud_rate_master = 
	     irlap_frame -> info [IRLAP_NEG_SNRM_PARAM_OFF + 2];
	   negotation_param.state = NEG_WAIT_FOR_UA;
	 } else {
	   printf ("\t==> IrLAP: SNRM-CMD: speed in 2 bytes! <==\n");
	   printf ("\t\t==> IrLAP: my max. speed is 115.2 kbps! <==\n");
	   negotation_param.baud_rate_master = 
	     irlap_frame -> info [IRLAP_NEG_SNRM_PARAM_OFF + 3];
	 } 
       } else {
	 printf ("\t==> IrLAP: SNRM-CMD: "
		 "No baud rate dictate in 1. byte! <==\n");
       }
     }
   } else {
     /* UA -rsp */
     negotation_param.baud_rate_client = irlap_frame -> info [10];
     negotation_param.state = NEG_UA_OK;
   
     /* now compute the new speed */
     tmp = 
       negotation_param.baud_rate_master & negotation_param.baud_rate_client;
     if ( tmp & 0x01 ) {
       new_speed = speed_list[i];
       m = 0;
     }
     for ( i=1; i < 7; i++) {
       tmp = tmp >> 1;
       if ( tmp & 0x01 ) {
	 new_speed = speed_list[i];
	 m = i;
       }
     }
     if ( m > 5 ) {
       /* not defined as B... on my system */
       printf ("Sorry: Required speed (%s baud) not supported!\n",
	       speed_name[m]);
     } else {
       printf ("New speed is: %s\n", speed_name[m]);
       /* now set the new speed */
       tcgetattr (port_fd, &set);
       cfsetospeed(&set, new_speed);
       cfsetispeed(&set, new_speed);
       tcsetattr(port_fd, TCSANOW, &set);
       tcgetattr(port_fd, &set);
       if ( (cfgetospeed(&set) != new_speed) ||
	    (cfgetispeed(&set) != new_speed) ) {
	 printf ("New speed is not set!\n");
       }
     }
   }
}

/*
 * decode_irlap_frame
 * ------------------
 * - decode the irlap-c-field
 */
void decode_irlap_frame ( IRLAP_FRAME * irlap_frame) 
{
  BYTE cmd = irlap_frame -> a & 0x01;
  BYTE poll = irlap_frame -> c & (IRLAP_PF_BIT_MASK);
  BYTE adr = (irlap_frame -> a) >> 1;
  
  switch ( irlap_frame -> c & IRLAP_C_MASK ) {
  case IRLAP_U_FRAME:
    printf ("U-Frame:\tAdr: %02X Nr: %02X ", adr,
	    (irlap_frame -> c & 0xE0) >> 5);
    printf ("P/F: %X \t\t", poll >> 4);
    switch (irlap_frame -> c & IRLAP_PF_BIT_CLR_MASK ) {
    case IRLAP_SNRM_RNRM:
      /* SNRM cmd/RNRM response */
      if ( cmd ) {
	printf ("SNRM cmd\n");
	switch (connection.irlap_state) {
	case IRLAP_DISC:
	  printf ("\t==> IrLAP in DISC but SNRM-CMD? <==\n");
	  break;
	case IRLAP_NDM:
	  printf ("\t==> Start IrLAP - negotation <==\n");
	  negotation ( irlap_frame );
	  break;
	case IRLAP_NRM:
	  printf ("\t==> IrLAP in NRM but SNRM-CMD? <==\n");
	  break;
	default:
	  printf ("\t==> IrLAP in not defined state but SNRM-CMD? <==\n");
	}
      } else {
	printf ("RNRM response\n"); 
      }
      break;
    case IRLAP_DISC_RD:
      /* DISC cmd/RD response */
      if ( cmd ) {
	printf ("DISC cmd\n"); 
      } else {
	printf ("RD response\n");
      }
      break;
    case IRLAP_UI_UI:
      /* UI cmd/UI response */
      if ( cmd ) {
	printf ("UI cmd\n");
      } else {
	printf ("UI response\n");
      }
      break;
    case IRLAP_XID_CMD:
      /* XID cmd */
      printf ("XID cmd\n");
      if ( connection.irlap_state == IRLAP_DISC ) {
	connection.irlap_state = IRLAP_NDM;
	printf ("\t==> Set IrLAP-state to:" 
		"NDM (normal disconnected mode) <==\n");
      }
      break;
    case IRLAP_TEST_TEST:
      /* Test cmd/response */
      if ( cmd ) {
	printf ("TEST cmd\n");
      } else {
	printf ("TEST response\n");
      }
      break;
    case IRLAP_UA_RSP:
      /* UA response */
      printf ("UA response\n");
      switch (connection.irlap_state) {
      case IRLAP_DISC:
	printf ("\t==> IrLAP in DISC but UA-RESPONSE? <==\n");
	break;
      case IRLAP_NDM:
	if ( negotation_param.state == NEG_WAIT_FOR_UA ) {
	  printf ("\t==> Continue IrLAP - negotation! <==\n");
	  negotation ( irlap_frame );
	} else {
	  printf ("\t==> IrLAP in NDM but not wait for  negotation! <==\n");
	}
	break;
      case IRLAP_NRM:
	printf ("\t==> UA: IrLAP was/is in NRM ... <==\n");
	break;
      default:
	printf ("\t==> IrLAP in not defined state but UA-RESPONSE? <==\n");
      }
      break;
    case IRLAP_FRMR_RSP:
      /* FRMR response */
      printf ("FRMR response\n");
      break;
    case IRLAP_DM_RSP:
      /* DM response */
      printf ("DM response\n");
      break;
    case IRLAP_XID_RSP:
      /* XID response */
      printf ("XID response\n");
      break;
    default:
      /* unknown cmd/response */
      printf ("Unknown IrLAP-U-Frame\n");
    } /* end of irlap-u-frames */
    break;
  case IRLAP_S_FRAME:
    printf ("S-Frame:\tAdr: %02X Nr: %02X ", adr,
	    (irlap_frame -> c & 0xE0) >> 5);
    printf ("P/F: %X \t\t", poll >> 4);
    switch ( irlap_frame -> c & 
	     (IRLAP_PF_BIT_CLR_MASK & IRLAP_Nr_CLR_MASK) ) {
    case IRLAP_RR:
      /* RR command/response */
      if ( cmd ) {
	printf ("RR cmd\n");
      } else {
	printf ("RR response\n");
      }
      break;
    case IRLAP_RNR:
      /* RNR cmd/response */
      printf ("RNR cmd/response\n");
      break;
    case IRLAP_REJ:
      /* REJ cmd/response */
      printf ("REJ cmd/response\n");
      break;
    case IRLAP_SREJ:
      /* SREJ cmd/response */
      printf ("SREJ cmd/response\n");
      break;
    default:
      /* unknown cmd/response */
      printf ("Unknown IrLAP-S-Frame\n");
    } /* end of irlap-s-frames */
    break;
  default:
    printf ("I-Frame:\tAdr: %02X Nr: %02X ", adr,
	    (irlap_frame -> c & 0xE0) >> 5);
    printf ("\tP/F: %X\t", poll >> 4);
    printf ("Ns: %02X \t", (irlap_frame -> c & 0x0E) >> 1);
    printf ("Information\n");
  }
}

/* unwrap_raw_frame
 * ----------------
 * - put raw-frame-datas in irlap-frame
 */
void unwrap_raw_frame ( RAW_FRAME * raw_frame ) 
{
  IRLAP_FRAME irlap_frame;
  BYTE * info_ptr;
  
  bzero ( &irlap_frame, sizeof(irlap_frame) );

  irlap_frame.a = raw_frame -> buf[IRLAP_A_OFF];
  irlap_frame.c = raw_frame -> buf[IRLAP_C_OFF];
  irlap_frame.fcs  = raw_frame -> buf [(raw_frame -> length) -2] << 8;
  irlap_frame.fcs |= raw_frame -> buf [(raw_frame -> length) -1];
  
  irlap_frame.info_length = raw_frame -> length - 
    IRLAP_A_LENGTH - IRLAP_C_LENGTH - IRLAP_FCS_LENGTH;

  info_ptr = g_malloc ( irlap_frame.info_length * sizeof (BYTE) );
  memcpy ( info_ptr, &(raw_frame -> buf[IRLAP_I_OFF]),
	   irlap_frame.info_length);
  irlap_frame.info = info_ptr;
  decode_irlap_frame ( &irlap_frame );
  g_free(info_ptr);
  
  printf ("IrLAP: FCS: %04X \n\n", irlap_frame.fcs );

}

/*
 * cleanup_termios
 * ---------------
 * - call on some signals from signalhandler
 * - before end set the original setting
 */
void cleanup_termios ( int signal )
{
  tcsetattr(port_fd, TCSANOW, &old_port_sets);
  exit (0);
}

/*
 * init_port
 * ---------
 * - get original port-settings and store it
 * - set new port-settings
 */
void init_port ( void ) 
{
  struct termios new_port_sets;
  
  /* get the original settings and store it for restore */
  tcgetattr( port_fd, &old_port_sets);

  /* init new settings */
  bzero(&new_port_sets, sizeof(new_port_sets));
  new_port_sets.c_cflag = INIT_BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
  new_port_sets.c_iflag = IGNBRK | IGNPAR;
  new_port_sets.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  new_port_sets.c_lflag = 0;
        
  /* inter-character timer unused x0.1s */
  new_port_sets.c_cc[VTIME]    = 0;
  /* blocking read until 5 chars received */
  new_port_sets.c_cc[VMIN]     = 1;

  /* set the port now */
  tcflush(port_fd, TCIFLUSH);
  tcsetattr(port_fd, TCSANOW, &new_port_sets);
}

/*
 * main
 * ----
 * - output-file and port open
 * - get/set port settings
 * - dataread-loop
 */
int main( int argc, char * argv[] )
{
  struct sigaction sact;	                         /* signalhandle    */
  GString *outfile_name = g_string_new(DEFAULT_OUTFILE); /* outfile name    */
  FILE *outfile;		                         /* outfile FILE    */
  fd_set ready;                                          /* for select      */
  struct timeval timeout;	                         /* select-timeout  */
  int no_timeout        = 1;	                         /* timeout reached */
  int nr_read           = 0;	                         /* byte readed     */
  unsigned char in_buffer[255];	                         /* temp. buffer    */
  RAW_FRAME raw_frame;		                         /* a raw frame     */
  int is_inframe        = FALSE;                         /* loop in frame   */
  int frame_complete    = FALSE;                         /* frame complete  */
  int done              = 0;	                         /* loop control    */
  int i                 = 0;			         /* for-index       */
  int nr_of_bytes       = 0;	                         /* total bytes     */
  int nr_of_frames      = 0;	                         /* total frames    */
  int irlap_bytes       = 0;                             /* w/o BOF etc.    */
  BYTE must_escaped     = FALSE;                         /* after CE = 0x07 */
  U16 fcsrx             = INIT_FCS;                      /* for fcs-comput. */
  BYTE nr_esc           = 0;	                         /* nr of CE's      */
//CONNECT_STATISTIC statistic;                           /* statistic       */
//OUTPUT_PARAMS output_params;                           /* output-control  */

  /*
   * Open the serial device
   */
  if ( (port_fd = open( MODEMDEVICE, O_NONBLOCK | O_RDWR)) < 0 ) {
    printf ( "Failed to open %s!\n", MODEMDEVICE);
    exit(1);
  }

  /*
   * get/set device fd flags
   */
  if ( (initfdflags = fcntl( port_fd, F_GETFL)) == -1 ) {
    printf ( "Couldn't get device fd flags for: %s!", MODEMDEVICE);
    exit(1);
  }
  initfdflags &= ~O_NONBLOCK;
  fcntl( port_fd, F_SETFL, initfdflags);

  init_port();

  /*
   * set signal-handler
   */
  sact.sa_handler = cleanup_termios;
  sigaction( SIGHUP,  &sact, NULL);
  sigaction( SIGINT,  &sact, NULL);
  sigaction( SIGPIPE, &sact, NULL);
  sigaction( SIGTERM, &sact, NULL);
  
  /*
   *  Set device for non-blocking reads.
   */
  if ( fcntl( port_fd, F_SETFL, initfdflags | O_NONBLOCK) == -1) {
    printf ("Couldn't set device to non-blocking mode (%s)!\n", MODEMDEVICE);
    exit(1);
  }
  
  /*
   * output-file-handling
   */
  if ( argc < 2 ) {
    printf ("Use default OutPutFile: %s\n", outfile_name -> str);
  } else {
    g_string_assign(outfile_name, argv[1]);
    g_string_append(outfile_name, OUTFILE_SUFFIX);
    printf ("Use OutPutFile: %s\n", outfile_name -> str);
  }
  if ( (outfile = fopen( outfile_name -> str, "wb")) == NULL ) {
    printf ("Failed to open OutPutFile: %s\n", outfile_name -> str);
    exit (1);
  }

  /*
   * initial timeout
   * ---------------
   * - wait 15 seconds
   */
  timeout.tv_sec  = INITIAL_TIMEOUT;
  timeout.tv_usec =  0;
  
  /*
   * init raw-frame, negotation-struct, connection
   */
  bzero(&raw_frame, sizeof(raw_frame));
  bzero(&negotation_param, sizeof(negotation_param));
  bzero(&connection, sizeof(connection));

  /* hope we have no irda traffic in moment */
  connection.irlap_state = IRLAP_DISC;
  
  /*
   * now make the data-read-loop
   */
  do {
    FD_ZERO(&ready);
    FD_SET(port_fd, &ready);
    no_timeout = select(port_fd + 1, &ready, NULL, NULL, &timeout);
    if ( FD_ISSET(port_fd, &ready) ) {
      /* data on port - returns after 16 chars have been input */
      nr_read = read(port_fd, in_buffer, 16);
      nr_of_bytes = nr_of_bytes + nr_read;
      if ( nr_read >= 1 ) {
	/* write all readed bytes to file */
	fwrite( in_buffer, 1, nr_read, outfile);
	/* process every single byte */
	for (i=0; i < nr_read; i++) {
	  if (raw_frame.length < (4096-1) ) {
	    switch ( in_buffer[i] ) {
	    case IRLAP_XBOF:
	      if ( is_inframe ) {
		/*
		 * no - its not a XBOF we are inside a frame 
		 * broadcast in xid for example
		 */
		irlap_bytes++;
		fcsrx = IR_FCS(fcsrx, in_buffer[i]);
		is_inframe = TRUE;
		raw_frame.buf[raw_frame.length] = in_buffer[i];
	      }
	      break;
	    case IRLAP_BOF:
	      is_inframe = TRUE;
	      break;
	    case IRLAP_EOF:
	      frame_complete = TRUE;
	      is_inframe = FALSE;
	      break;
	    case IRLAP_CE:
	      must_escaped = TRUE;
	      is_inframe = TRUE;
	      nr_esc++;
	      break;
	    default:
	      is_inframe = TRUE;
	      if ( must_escaped ) {
		fcsrx = IR_FCS(fcsrx, (in_buffer[i] ^ IRLAP_ESC_MASK) );
		raw_frame.buf[raw_frame.length] = 
		  (in_buffer[i]^IRLAP_ESC_MASK);
		must_escaped = FALSE;
	      } else {
		fcsrx = IR_FCS(fcsrx, in_buffer[i]);
		raw_frame.buf[raw_frame.length] = in_buffer[i];
	      }
	      irlap_bytes++;
	    }
	    raw_frame.length = irlap_bytes;
	    if ( frame_complete ) {
	      /* now raw-frame is complete */
	      if ( fcsrx != GOOD_FCS ) {
		printf (" **** !!! FCS-ERROR !!! ****\n");
	      } else {
		unwrap_raw_frame ( &raw_frame );
	      }
	      nr_of_frames++;
	      /* reset all to defaults     */
	      raw_frame.length   = 0;
	      raw_frame.a_offset = 0;
	      frame_complete     = FALSE;
	      fcsrx              = INIT_FCS;
	      nr_esc             = 0;
	      irlap_bytes        = 0;
	    }
	  }
	}
      } else {
	printf ("No data to read - why?\n");
	done = 1;
      }
    }
    /*
     * reset timeout - wait 5 seconds
     */
    timeout.tv_sec  = DEFAULT_TIMEOUT;
    timeout.tv_usec = 0;
    if ( ! no_timeout ) {
      printf ("TimeOut!\nConnection summary:\n-------------------\n");
      printf ("Total nr of received bytes : %i\n", nr_of_bytes);
      printf ("Total nr of received frames: %i\n", nr_of_frames);
      done = 1;
    }
  } while ( ! done );

  g_string_free(outfile_name, TRUE);
  fclose (outfile);
  tcsetattr(port_fd, TCSANOW, &old_port_sets);
  exit(0);
}
