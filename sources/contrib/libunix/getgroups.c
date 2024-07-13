#include <mlvalues.h>
#include <alloc.h>

#ifdef HAS_GETGROUPS

#include "unix.h"

value unix_getgroups()           /* ML */
{
  int n;
  value res;
  int i;
  long ngroups_max;

#ifdef NGROUPS
  gid_t gidset[NGROUPS];
  ngroups_max = NGROUPS;
#else
  ngroups_max = sysconf(_SC_NGROUPS_MAX) + 1;
  gid_t gidset[ngroups_max];
#endif /* NGROUPS */

  n = getgroups(ngroups_max, gidset);
  if (n == -1)
  	uerror("getgroups", Nothing);
  res = alloc_tuple(n);
  for (i = 0; i < n; i++)
    Field(res, i) = Val_int(gidset[i]);
  return res;
}

#else

value unix_getgroups() { invalid_argument("getgroups not implemented"); }

#endif
