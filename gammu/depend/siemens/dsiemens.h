/* (c) by Walek */

#ifndef at_siemens_h
#define at_siemens_h

typedef struct {
	int	ChFreq;
	int	RxLev;
	int	NB;
} GSM_SAT_ChInfo;

typedef struct {
	gboolean		BA_used;
	gboolean		DTX_used;
	gboolean		MeasValid;
	int		RXLEV_FullServicingCell;
	int		RXLEV_SubServicingCell;
	unsigned char	RXQUAL_FullServicingCell;
	unsigned char	RXQUAL_SubServicingCell;
	unsigned char	NO_NCELL_M;
	GSM_SAT_ChInfo	NeighbourCell[8];
} GSM_SAT_Measure_results;

void ATSIEMENSSATNetmon(int argc, char *argv[]);
void ATSIEMENSNetmonitor(int argc, char *argv[]);
void ATSIEMENSActivateNetmon(int argc, char *argv[]);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
