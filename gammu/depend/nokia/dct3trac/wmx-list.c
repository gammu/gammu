/**
 * Decode trace type using text file
 * wumpus 2003 -- www.blacksphere.tk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "gammu.h" //include added bacause of MS VC 2005
#include "wmx-util.h"
#include "wmx-list.h"

static void wmx_tracelist_init(struct wmx_tracelist *tl)
{
	tl->entries = 0;
	tl->max = 10;
	tl->records = malloc(tl->max * sizeof(struct wmx_tracetype));
}

static void wmx_tracelist_add(struct wmx_tracelist *tl, int type, char *desc)
{
	/* Possibly expand list */
	if(tl->entries == tl->max) {
		tl->max *= 2;
		tl->records = realloc(tl->records, tl->max * sizeof(struct wmx_tracetype));
	}
	/* Add record */
	tl->records[tl->entries].type = type;
	tl->records[tl->entries].desc = strdup(desc);
	/* Increase number of entries */
	tl->entries++;
	//printf("%04x %04x\n", tl->entries, tl->max);
}

static void wmx_tracelist_free(struct wmx_tracelist *tl)
{
	int x;

	for(x=0; x<tl->entries; x++) free(tl->records[x].desc);
	free(tl->records);
	tl->entries = tl->max = 0;
	tl->records = NULL;
}

/** Comparision function for bsearch */
static int compar(const void *a, const void *b)
{
	const int typea = ((const struct wmx_tracetype*)a)->type;
	const int typeb = ((const struct wmx_tracetype*)b)->type;
	if(typea < typeb) return -1;
	if(typea > typeb) return 1;
	return 0;
}

/**
 * Look up debug trace type
 */
static struct wmx_tracetype *wmx_tracelist_search(struct wmx_tracelist *tl, int type)
{
	/*
	int x;
	for(x=0; x<tl->entries; x++)
		if(tl->records[x].type == type)
			return &tl->records[x];
	return NULL;
	*/

	struct wmx_tracetype key;

	key.type = type;
	return bsearch(&key, tl->records, tl->entries, sizeof(struct wmx_tracetype), &compar);
}

struct wmx_tracestruct *wmx_tracestruct_load(char *listfile)
{
	char 			data[256];
	FILE 			*f;
	struct wmx_tracestruct 	*tl;
	int 			tpe;

	f = fopen(listfile, "r");

	if(f == NULL) return NULL;
	printf("Loading\n");

	tl = malloc(sizeof(struct wmx_tracestruct));
	wmx_tracelist_init(&tl->minors);
	wmx_tracelist_init(&tl->majors);

	while(fgets(data, sizeof(data), f)) {
		int l = strlen(data);
		if(l > 0 && data[l-1]=='\n') l--;
		data[l] = 0;
		if(l < 6)
			/* Empty/invalid line */
			continue;
		if(data[2] == 'X' && data[3] == 'X') {
			sscanf(data, "%02X", &tpe);
			wmx_tracelist_add(&tl->majors, tpe, &data[5]);
		} else {
			sscanf(data, "%04X", &tpe);
			wmx_tracelist_add(&tl->minors, tpe, &data[5]);
		}
	}
	fclose(f);
	return tl;
}


struct wmx_tracetype *wmx_tracestruct_querymajor(struct wmx_tracestruct * ts, int type) {
	return wmx_tracelist_search(&ts->majors, type);
}

struct wmx_tracetype *wmx_tracestruct_queryminor(struct wmx_tracestruct * ts, int type) {
	return wmx_tracelist_search(&ts->minors, type);
}

void wmx_tracestruct_free(struct wmx_tracestruct * ts) {
	wmx_tracelist_free(&ts->minors);
	wmx_tracelist_free(&ts->majors);
	free(ts);
}

/*
main() {
	int x;
	struct wmx_tracestruct * tstruct= wmx_tracestruct_load(
		"/home/orion/projects/blacksphere/tables/nhm5_587.txt");

	printf("%s\n", wmx_tracestruct_queryminor(tstruct, 0x1802)->desc);
}
*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
