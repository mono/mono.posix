#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <cerrno>
#include <dirent.h>
#include <cstring>
#include "mph.hh" /* Don't remove or move after map.h! Works around issues with Android SDK unified headers */
#include "map.hh"

int
wifexited (int status)
{
	return WIFEXITED (status);
}

int
wexitstatus (int status)
{
	return WEXITSTATUS (status);
}

int
wifsignaled (int status)
{
	return WIFSIGNALED (status);
}

int
wtermsig (int status)
{
	return WTERMSIG (status);
}

int
wifstopped (int status)
{
	return WIFSTOPPED (status);
}

int
wstopsig (int status)
{
	return WSTOPSIG (status);
}

int
helper_Mono_Posix_Stat(const char *filename, int dereference,
	int *device,
	int *inode,
	int *mode,
	int *nlinks,
	int *uid,
	int *gid,
	int *rdev,
	int64_t *size,
	int64_t *blksize,
	int64_t *blocks,
	int64_t *atime,
	int64_t *mtime,
	int64_t *ctime
	)
{
	int ret;
	struct stat buf;
	
	if (!dereference) {
		ret = stat (filename, &buf);
	} else {
		ret = lstat (filename, &buf);
	}

	if (ret != 0) {
		return ret;
	}
	
	*device = buf.st_dev;
	*inode = buf.st_ino;
	*mode = buf.st_mode;
	*nlinks = buf.st_nlink;
	*uid = buf.st_uid;
	*gid = buf.st_gid;
	*rdev = buf.st_rdev;
	*size = buf.st_size;
	*blksize = buf.st_blksize;
	*blocks = buf.st_blocks;
	*atime = buf.st_atime;
	*mtime = buf.st_mtime;
	*ctime = buf.st_ctime;
	return 0;
}

char*
helper_Mono_Posix_GetUserName(int uid)
{
	struct passwd *p = getpwuid (uid);
	if (p == nullptr) {
		return nullptr;
	}

	return strdup (p->pw_name);
}

char*
helper_Mono_Posix_GetGroupName (int gid)
{
	struct group *p = getgrgid (gid);
	if (p == nullptr) {
		return nullptr;
	}

	return strdup (p->gr_name);
}

char*
helper_Mono_Posix_readdir (DIR* dir)
{
	if (dir == nullptr) {
		return nullptr;
	}

	struct dirent* e = readdir (dir);
	if (e == nullptr) {
		return nullptr;
	}

	return strdup (e->d_name);
}

#if HAVE_GETPWNAM_R
int
helper_Mono_Posix_getpwnamuid (int mode, char *in_name, int in_uid,
	char **account,
	char **password,
	int *uid,
	int *gid,
	char **name,
	char **home,
	char **shell
	)
{
	static constexpr size_t BUF_SIZE = 4096;

	struct passwd pw, *pwp;
	char buf[BUF_SIZE];
	int ret;

	if (mode == 0) {
		ret = getpwnam_r (in_name, &pw, buf, BUF_SIZE, &pwp);
	} else {
		ret = getpwuid_r (in_uid, &pw, buf, BUF_SIZE, &pwp);
	}

	if (ret == 0 && pwp == nullptr) {
		// Don't know why this happens, but it does.
		// ret == 0, errno == 0, but no record was found.
		ret = ENOENT;
	}

	if (ret != 0) {
		*account = nullptr; // prevent marshalling unset pointers
		*password = nullptr;
		*uid = 0;
		*gid = 0;
		*name = nullptr;
		*home = nullptr;
		*shell = nullptr;
		return ret;
	}

	*account = pwp->pw_name;
	*password = pwp->pw_passwd;
	*uid = pwp->pw_uid;
	*gid = pwp->pw_gid;
#if defined (HAVE_STRUCT_PASSWD_PW_GECOS)
	*name = pwp->pw_gecos;
#else
	*name = nullptr;
#endif
	*home = pwp->pw_dir;
	*shell = pwp->pw_shell;

	return 0;
}
#endif  /* def HAVE_GETPWNAM_R */

