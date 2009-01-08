/* Sample code to dump all feature codes and their descriptions */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	GSM_Feature feature;
	bool numeric = false;
	int rc = 0;
	const char *featurestring;

	if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
		numeric = true;
	}

	for (feature = F_CAL33; feature < F_LAST_VALUE; feature++) {
		featurestring = GSM_FeatureToString(feature);
		if (featurestring == NULL) {
			fprintf(stderr, "Unknown feature message for %d!\n", feature);
			rc = 1;
		} else {
			if (feature != GSM_FeatureFromString(featurestring)) {
				fprintf(stderr, "Could not map string %s back to %d!\n", featurestring, feature);
				rc = 2;
			}
		}
		if (numeric) {
			printf("%d. %s\n", feature, featurestring);
		} else {
			printf("# %s\n", featurestring);
		}
	}

	return rc;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

