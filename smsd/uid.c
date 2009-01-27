#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>

#include "uid.h"

/**
 * Sets UID.
 */
bool set_uid_gid(SMSD_Parameters *params)
{
    /* gid is set allways (either through fill_uid or by user */
    if (setgid(params->gid) != 0)
        return false;

    /* Only set group */
    if (params->uid == -1)
        return true;

    if (initgroups(params->user, params->gid) != 0)
        return false;

    if (setuid(params->uid) != 0)
        return false;

    if (seteuid(params->uid) != 0)
        return false;

    return true;
}

/**
 * Sets GID.
 */
bool set_gid(int gid)
{
    return setgid(gid) == 0;
}

/**
 * Gets UID from string.
 */
bool fill_uid(SMSD_Parameters *params, const char *name)
{
    struct passwd *pwd;
    char *endptr;
    long uid;

    pwd = getpwnam(name);

    if (pwd == NULL) {
        /* Try to handle it as a number */
        uid = strtol(name, &endptr, 10);
        if (*endptr == 0 && uid > 0) {
            pwd = getpwuid(uid);
        }
    }

    if (pwd != NULL) {
        params->uid = pwd->pw_uid;
        params->user = strdup(pwd->pw_name);
        if (params->gid == -1) {
            params->gid = pwd->pw_gid;
        }
        return true;
    }

    return false;
}

/**
 * Gets GID from string.
 */
bool fill_gid(SMSD_Parameters *params, const char *name)
{
    struct group *grp;
    char *endptr;
    long gid;

    grp = getgrnam(name);

    /* Try to handle it as a number */
    if (grp == NULL) {
        gid = strtol(name, &endptr, 10);
        if (*endptr == 0 && gid > 0) {
            grp = getgrgid(gid);
        }
    }

    if (grp != NULL) {
        params->gid = grp->gr_gid;
        params->group = strdup(grp->gr_name);
        return true;
    }

    return false;
}

