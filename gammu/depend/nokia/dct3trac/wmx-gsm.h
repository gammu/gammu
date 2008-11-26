#ifndef WMX_GSM_INCLUDED
#define WMX_GSM_INCLUDED

typedef struct GSMDecoder_struct {
	FILE *xmlout;
} GSMDecoder;

typedef struct GSMDecoder_l1l2data_struct {
	/* 0 receive
	   1 send
	*/
	int 		tx;

	#define GSMDECODER_RECEIVE 0
	#define GSMDECODER_SEND    1

	/* send and receive */
	int 		ch; 		/* logical channel */

	/* receive only: */
	int 		bsic;		/* base station identity code */
	int 		arfcn;		/* physical channel */
	int 		seq;		/* sequence number */
	short 		timeshift;	/* diff time with sequence number */
	int 		err;		/* error flag */
} GSMDecoder_l1l2data;

GSMDecoder *GSMDecoder_new(void);

void GSMDecoder_free		(GSMDecoder *self);
void GSMDecoder_xmlout		(GSMDecoder *self, FILE *f);

void GSMDecoder_L3packet	(GSMDecoder *self, GSMDecoder_l1l2data *l1, unsigned char *buffer, size_t length);
void GSMDecoder_L2short_packet	(GSMDecoder *self, GSMDecoder_l1l2data *l1, unsigned char *buffer, size_t length);
void GSMDecoder_L2packet	(GSMDecoder *self, GSMDecoder_l1l2data *l1, unsigned char *buffer, size_t length);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
