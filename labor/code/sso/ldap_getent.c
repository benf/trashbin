#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdint.h>
/* dlopen interface */
#include <dlfcn.h>

/* defines struct passwd */
#include <pwd.h>
/* defines struct group */
#include <grp.h>

/*  set/end..ent functions */
typedef int32_t (*setXXent_t) (void);
typedef setXXent_t endXXent_t;

typedef int32_t (*getXXent_t)(void *result, char *buffer,
    size_t buflen, int *errnop);

/* returns pointer to requested function + error handling */
void *get_function(void *handle, const char *name);
void  fetch_data(void *handle, const char *db);

int main() {
  void *handle = dlopen("libnss_ldap.so.2", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "Cannon open library: %s\n", dlerror());
    return 1;
  }
  /* pw = passwd */
  fetch_data(handle, "pw");
  /* gr = group */
  fetch_data(handle, "gr");
  dlclose(handle);
  return 0;
}

void *get_function(void *handle, const char *name) {
  void *ret = dlsym(handle, name);
  const char *dlsym_error = dlerror();
  if (dlsym_error) {
    fprintf(stderr, "Cannot load symbol %s: %s\n",
        name, dlsym_error);
    exit(2);
  }
  return ret;
}

void fetch_data(void *handle, const char *db) {
  uint32_t bufsize = 1024;
  size_t struct_size;
  int32_t errnop;

  if (strncmp(db,"pw", 2) == 0)
    struct_size = sizeof(struct passwd);
  else if (strncmp(db,"gr", 2) == 0)
    struct_size = sizeof(struct group);
  else return;

  char setent[] = "_nss_ldap_setXXent";
  char getent[] = "_nss_ldap_getXXent_r";
  char endent[] = "_nss_ldap_endXXent";
  /* replace XX's */
  strncpy(setent+13, db, 2);
  strncpy(getent+13, db, 2);
  strncpy(endent+13, db, 2);

  setXXent_t ldap_setent   = get_function(handle, setent);
  getXXent_t ldap_getent_r = get_function(handle, getent);
  endXXent_t ldap_endent   = get_function(handle, endent);

  void *data = malloc(struct_size);
  void *buf  = malloc(bufsize);

  ldap_setent();
  while (ldap_getent_r(data, buf, bufsize, &errnop)) {
    if (errnop == ERANGE) {
      buf = realloc(buf, bufsize<<=1);
      errnop = 0;
      continue;
    }
    if (strncmp(db,"pw", 2) == 0) {
      struct passwd *pw = (struct passwd*) data;
      printf("%s:%s:%d:%d:%s:%s:%s\n",
        pw->pw_name, pw->pw_passwd, pw->pw_uid, pw->pw_gid,
        pw->pw_gecos, pw->pw_dir, pw->pw_shell);
    }
    if (strncmp(db,"gr", 2) == 0) {
      struct group *grp = (struct group*) data;
      printf("%s:%d:", grp->gr_name, grp->gr_gid);
      uint32_t i = 0;
      while (grp->gr_mem[i]) {
        if (i > 0) putchar(',');
        printf(grp->gr_mem[i++]);
      }
      putchar('\n');
    }
  }
  ldap_endent();

  free(data);
  free(buf);
}
