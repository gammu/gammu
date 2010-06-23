/* xsiemens.c */
/* Siemens service code generator */
/* (C) Ivan Gorinov, 30.09.2002 */

#include "chiffre.h"

unsigned char key1[4] = {0x57, 0xF9, 0x3A, 0xC1};  /* Level 1 key */
unsigned char key2[4] = {0xA4, 0xD3, 0x67, 0x9C};  /* Level 2 key */

/* 4-bit substitution boxes */
unsigned char sbox_h[0x10] =
{0x09, 0x02, 0x04, 0x06, 0x08, 0x07, 0x05, 0x03, 0x01, 0x00,
 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
unsigned char sbox_l[0x10] =
{0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

unsigned char sbox[0x100];  /* 8-bit substitution box */

char digit[] = "0123456789ABCDEF";

/* Convert hex digit to binary value */
int from_hex(int c)
{
	int i;

	for(i = 0; i < 0x10; i++) if(c == digit[i]) break;
	if (i >= 0x10) i = -1;
    	return i;
}

/* Convert binary value to string (decimal) */
int bin2dec(char *s, int n, unsigned long l)
{
    	int c, i, j;

    	j    = n;
    	s[j] = 0;
	for(i = 0; i < n; i++) {
		c = l % 10;
		l /= 10;
		c = digit[c];
		s[--j] = c;
    	}
    	return j;
}

/* Convert two 4-bit boxes to one 8-bit */
int initbox(void)
{
    	int i, j, k, x;

    	k = 0;
    	for(i = 0; i < 0x10; i++) {
		x = sbox_h[i] << 4;
		for(j = 0; j < 0x10; j++) {
	    		sbox[k] = sbox_l[j] | x;
	    		k++;
		}
    	}
    	return 0;
}

/* Substitution */
void subst(unsigned char *b, unsigned char *a, int n)
{
    	int i;

    	for(i = 0; i < n; i++) b[i] = sbox[a[i]];
}

/* Xor N bytes with Z, inverting every odd byte */
void xxstr(unsigned char *b, unsigned char *a, int n, int z)
{
    	int i;

    	for(i = 0; i < n; i++) {
		if(i & 1) {
	    		b[i] = (~a[i]) ^ z;
		} else {
	    		b[i] = a[i] ^ z;
		}
    	}
}

/* Encryption (1 iteration) */
void ec_f(unsigned char *a, int n, int z)
{
    	unsigned char 	bc[0x20], bs[0x10], bx[0x10];
    	int 		n2, i;

    	for(i = 0; i < n; i++) bc[i] = a[i];
    	n2 = n >> 1;
    	subst(bs, a, n2);
    	xxstr(bx, bs, n2, z);
    	for(i = 0; i < n2; i++) {
		a[i] = bx[i] ^ bc[i+n2];
		a[i+n2] = bc[i];
    	}
}

void ec_f_bak(unsigned char *a, int n, int z)
{
    	unsigned char 	bc[0x20], bs[0x10], bx[0x10];
    	int 		n2, i;

    	n2 = n >> 1;
   	for(i = 0; i < n2; i++) bc[i+n2] = a[i];
    	subst(bs, bc + n2, n2);
    	xxstr(bx, bs, n2, z);
    	for(i = 0; i < n2; i++) bc[i] = bx[i] ^ a[i+n2];
    	for(i = 0; i < n; i++) a[i] = bc[i];
}

/* Decryption (1 iteration) */
void dc_f(unsigned char *a, int n, int z)
{
    	unsigned char 	bc[0x20], bs[0x10], bx[0x10];
    	int 		n2, i;

    	for(i = 0; i < n; i++) bc[i] = a[i];
    	n2 = n >> 1;
    	subst(bs, a + n2, n2);
    	xxstr(bx, bs, n2, z);
    	for(i = 0; i < n2; i++) {
		a[i] = bc[i+n2];
		a[i+n2] = bx[i] ^ bc[i];
    	}
}

/* Encryption (IMSI -> code) */
void ss_encrypt(unsigned char *a, int n, unsigned char *key)
{
    	int i;

    	for(i = 0; i < 4; i++) ec_f_bak(a, n, key[i]);
}

/* Decryption (code -> IMSI) */
void ss_decrypt(unsigned char *a, int n, unsigned char *key)
{
    	int i;

    	for(i = 0; i < 4; i++) dc_f(a, n, key[3-i]);
}

int siemens_code(char *str_imsi, char *str_code, int level)
{
    	unsigned char 	array[8];
    	unsigned char 	xcode[8];
    	unsigned char 	test[8];
    	unsigned long 	l0, l1;
    	unsigned char 	*key;
    	int 		x, c, i, j;

    	if(!str_code) return -1;
    	str_code[0] = 0;

    	initbox();

    	switch(level) {
		case 1: key = key1; break;
		case 2: key = key2; break;
		default: return -1;
    	}

    	/* Check the length of IMSI */
    	for(i = 0; i < 32; i++) if(!str_imsi[i]) break;
    	if(i == 0) return -1;
    	if(i >= 32) return -1;

    	/* Convert last 12 digits to 6 bytes [2-7] */
    	for(j = 7; j >= 2; j--) {
		if (i == 0) {
	    		c = 0x0F;
		} else {
	    		c = str_imsi[--i];
	    		c = from_hex(c);
	    		if(c < 0) return -1;
		}
		x = c;
		if(i == 0) {
	    		c = 0x0F;
		} else {
	    		c = str_imsi[--i];
	    		c = from_hex(c);
	    		if(c < 0) return -1;
		}
		x = (x << 4) | c;
		array[j] = x;
    	}

    	/* This byte is never checked by firmware */
    	array[1] = 0;

    	/* Byte [0] <- checksum */
    	x = 0;
    	for(i = 1; i < 8; i++) x ^= array[i];
    	array[0] = x;

    	for(i = 0; i < 8; i++) xcode[i] = array[i];

    	/* Make the code */
    	ss_encrypt(xcode, 8, key);

    	/* The test */
    	for(i = 0; i < 8; i++) test[i] = xcode[i];
    	ss_decrypt(test, 8, key);
    	for(i = 0; i < 8; i++) if(test[i] != array[i]) break;
    	if(i < 8) return -1;

    	/* Convert to strings */
    	l0 = 0;
    	for(i = 3; i >= 0; i--) l0 = (l0 << 8) | xcode[i];

    	l1 = 0;
    	for(i = 7; i >= 4; i--) l1 = (l1 << 8) | xcode[i];

    	bin2dec(str_code, 10, l0);
    	bin2dec(str_code + 10, 10, l1);

    	return 0;
}
