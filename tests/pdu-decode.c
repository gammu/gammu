/*
 * Very simple PDU decoder, just parses header and is independent to
 * Gammu library. It is used for verification of Gammu test - anything
 * this fails to process should also fail in Gammu.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 16384

/**
 * Converts hex digit to number, returns -1 on failure.
 */
int hexdigit2number(const char digit) {
	if (digit >= '0' && digit <= '9') {
		return digit - '0';
	}
	if (digit >= 'a' && digit <= 'f') {
		return 0xa + digit - 'a';
	}
	if (digit >= 'A' && digit <= 'F') {
		return 0xa + digit - 'A';
	}
	printf("%c is not a digit!\n", digit);
	return -1;
}

/**
 * Converts hex string of arbitrary length to number, returns -1 on
 * failure.
 */
int hex2number(const char *buffer, const size_t len) {
	int result = 0, tmp;
	size_t pos;

	for (pos = 0; pos < len; pos++) {
		result = result << 4;
		tmp = hexdigit2number(buffer[pos]);
		if (tmp < 0) return tmp;
		result += tmp;
	}
	return result;
}

/**
 * Reads single number encoded in PDU.
 */
int pdu_get_number(const char *buffer, const int semioctet) {
	int length;
	int type;
	char *out;
	int i;

	length = hex2number(buffer, 2);
	if (semioctet) {
		if (length % 2) length++;
		length = length / 2 + 1;
	}
	printf("Number length = %d\n", length);

	if (length == 0) return 2;

	type = hex2number(buffer + 2, 2);
	printf("Number type = %d\n", type);

	out = (char *)malloc((2 * length) + 1);
	if (out == NULL) return -1;
	memset(out, 0, 2 * length);

	for (i = 0; i < (length - 1) * 2; i++) {
		if (!isxdigit(buffer[4 + i])) {
			printf("Non hex digit in PDU (%s)!\n",
					buffer + 4 +  i);
			free(out);
			return -1;
		}

		if (i % 2 == 0) {
			out[i + 1] = buffer[4 + i];
			if (out[i + 1] == 'F') {
				out[i + 1] = '\0';
			}
		} else {
			out[i - 1] = buffer[4 + i];
		}
	}
	printf("Number = %s\n", out);
	free(out);

	return (length * 2) + 2;
}

/**
 * Parses timestamp from PDU.
 */
int pdu_get_timestamp(const char *buffer) {
	int i;


	for (i = 0; i < 14; i++) {
		if (!isxdigit(buffer[i])) return -1;
	}

	printf("Date: %d-%d-%d %d:%d:%d TZ=%d\n",
			hex2number(buffer +  0, 2),
			hex2number(buffer +  2, 2),
			hex2number(buffer +  4, 2),
			hex2number(buffer +  6, 2),
			hex2number(buffer +  8, 2),
			hex2number(buffer + 10, 2),
			hex2number(buffer + 12, 2)
	      );


	return 14;
}

/**
 * Decodes textual PDU.
 */
int pdu_decode(const char *buffer) {
	/* Values */
	int type;
	int mr;
	/* Status variables */
	int pos = 0, ret;
	/* Message content */
	int submit = 0, deliver = 0, report = 0;
	int vpf = 0;
	int rp = 0;
	int udh = 0;
	int pid = 0;
	int dcs = 0;
	int vp = 0;
	int udhl = 0;
	int ud;
	int i;

	/* SMSC number */
	ret = pdu_get_number(buffer + pos, 0);
	if (ret < 0) return ret;
	pos += ret;

	/* Message type */
	type = hex2number(buffer + pos, 2);
	if (type < 0) return type;
	pos += 2;
	printf("Message type: %02x - ", type);

	switch (type & 0x3) {
		case 0:
			printf("Deliver");
			deliver = 1;
			break;
		case 1:
			printf("Submit");
			submit = 1;
			break;
		case 2:
			printf("Status report\n");
			report = 1;
			break;
		case 3:
			printf("Reserverd\n");
			return -1;
	}
	if (submit || deliver) {
		if (type & (1 << 7)) {
			rp = 1;
		}
		if (type & (1 << 6)) {
			udh = 1;
		}
	}
	if (rp) {
		printf(", Reply path set");
	}
	if (udh) {
		printf(", UDH included");
	}
	if (submit) {
		switch (type & (0x3 << 3)) {
			case 0:
				printf(", No VP");
				break;
			case 1:
				printf(", Reserved VP!\n");
				return -1;
			case 2:
				printf(", Relative VP");
				vpf = 2;
				break;
			case 3:
				printf(", Absolute VP");
				vpf = 3;
				break;
		}
	}
	printf("\n");

	/* Message reference (for submit) */
	if (submit || report) {
		mr = hex2number(buffer + pos, 2);
		if (mr < 0) return mr;
		pos += 2;
		printf("MR = 0x%02X\n", mr);
	}

	/* Address (sender for deliver, receiver for submit, recipient
	 * for report) */
	ret = pdu_get_number(buffer + pos, 1);
	if (ret < 0) return ret;
	pos += ret;

	if (report) {
		/* Timestamp */
		ret = pdu_get_timestamp(buffer + pos);
		if (ret < 0) return ret;
		pos += ret;

		/* SMSC timestamp */
		ret = pdu_get_timestamp(buffer + pos);
		if (ret < 0) return ret;
		pos += ret;
	}
	if (submit || deliver) {
		/* PID */
		pid = hex2number(buffer + pos, 2);
		if (pid < 0) return pid;
		pos += 2;
		printf("PID = 0x%02X\n", pid);

		/* DCS */
		dcs = hex2number(buffer + pos, 2);
		if (dcs < 0) return dcs;
		pos += 2;
		printf("DCS = 0x%02X\n", dcs);
	}
	if (submit) {
		/* Validity */
		if (vpf == 2 || vpf == 0) {
			vp = hex2number(buffer + pos, 2);
			if (vp < 0) return vp;
			pos += 2;
			printf("VP = 0x%02X\n", vp);
		} else if (vpf == 3) {
			ret = pdu_get_timestamp(buffer + pos);
			if (ret < 0) return ret;
			pos += ret;
		}
	}
	if (deliver) {
		/* SMSC timestamp */
		ret = pdu_get_timestamp(buffer + pos);
		if (ret < 0) return ret;
		pos += ret;
	}
	if (submit || deliver) {
		/* UD */
		udhl = hex2number(buffer + pos, 2);
		if (udhl < 0) return udhl;
		pos += 2;
		printf("UDL = 0x%02X\n", udhl);
		if ((dcs == 0) ||
			(((dcs & 0xC0) == 0xc0) && ((dcs & 4) != 4)) ||
			(((dcs & 0xf0) == 0xd0) && ((dcs & 4) != 4)) ||
			(((dcs & 0xf0) == 0xe0) && ((dcs & 4) != 4)) ||
			(((dcs & 0xf0) == 0xf0) && ((dcs & 4) != 4))
			) {
			if ((udhl * 7) % 8 != 0) {
				udhl = (udhl * 7) / 8;
				udhl++;
			} else {
				udhl = (udhl * 7) / 8;
			}
			printf("UDL[adjusted] = 0x%02X\n", udhl);
		}
		printf("data = ");
		for (i = 0; i < udhl; i++) {
			ud = hex2number(buffer + pos, 2);
			if (ud < 0) {
				printf("\nData too short!\n");
				return ud;
			}
			pos += 2;
			printf("%02X", ud);
		}
		printf("\n");
	}

	if (buffer[pos] != '\r' && buffer[pos] != '\n' && buffer[pos] != '\0') {
		printf("Did not reach end: %s\n", buffer + pos);
		return 1;
	}


	return 0;
}

int main(int argc, char **argv) {
	char buffer[BUFFER_SIZE];
	char *pos;
	FILE *f;
	size_t len;

	/* Check parameters */
	if (argc != 2) {
		printf("Not enough parameters!\nUsage: pdu-decode comm.dump\n");
		return 1;
	}

	/* Open file */
	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Could not open %s\n", argv[1]);
		return 1;
	}

	/* Read data */
	len = fread(buffer, 1, sizeof(buffer) - 1, f);
	if (!feof(f)) {
		printf("Could not read whole file %s\n", argv[1]);
		return 1;
	}
	/* Zero terminate data */
	buffer[len] = 0;

	/* Close file */
	fclose(f);

	pos = strchr(buffer, '\n');
	if (pos == NULL) return 2;
	pos++;

	pos = strchr(pos, '\n');
	if (pos == NULL) return 2;
	pos++;

	if (pdu_decode(pos) < 0) return 3;

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
