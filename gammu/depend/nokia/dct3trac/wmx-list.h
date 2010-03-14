#ifndef WMX_TRACELIST_INCLUDED
#define WMX_TRACELIST_INCLUDED

struct wmx_tracetype {
	int 			type;
	char 			*desc;
};

struct wmx_tracelist {
	int 			max, entries;
	struct wmx_tracetype 	*records;
};

struct wmx_tracestruct {
	struct wmx_tracelist 	majors, minors;
};

struct wmx_tracestruct 	*wmx_tracestruct_load(char *listfile);
struct wmx_tracetype 	*wmx_tracestruct_querymajor(struct wmx_tracestruct * ts, int type);
struct wmx_tracetype 	*wmx_tracestruct_queryminor(struct wmx_tracestruct * ts, int type);

void wmx_tracestruct_free(struct wmx_tracestruct * ts);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
