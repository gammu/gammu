#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>

#include "uid.h"

/**
 * Sets UID.
 */
bool set_uid(int uid)
{
    return setuid(uid) == 0;
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
int get_uid(const char *name)
{
    struct passwd *pwd;
    char *endptr;
    long uid;

    pwd = getpwnam(name);

    if (pwd != NULL) {
        return pwd->pw_uid;
    }

    /* Try to handle it as a number */
    uid = strtol(name, &endptr, 10);
    if (*endptr == 0 && uid > 0) {
        return uid;
    }

    return -1;
}

/**
 * Gets GID from string.
 */
int get_gid(const char *name)
{
    struct group *grp;
    char *endptr;
    long gid;

    grp = getgrnam(name);

    if (grp != NULL) {
        return grp->gr_gid;
    }

    /* Try to handle it as a number */
    gid = strtol(name, &endptr, 10);
    if (*endptr == 0 && gid > 0) {
        return gid;
    }

    return -1;
}

