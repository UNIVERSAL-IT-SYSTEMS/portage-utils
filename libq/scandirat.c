/*
 * Copyright 2005-2014 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 *
 * Copyright 2005-2010 Ned Ludd        - <solar@gentoo.org>
 * Copyright 2005-2014 Mike Frysinger  - <vapier@gentoo.org>
 */

#if !defined(HAVE_SCANDIRAT)
# if defined(__GLIBC__) && (__GLIBC__ << 8 | __GLIBC_MINOR__) > (2 << 8 | 14)
#  define HAVE_SCANDIRAT
# endif
#endif

#if !defined(HAVE_SCANDIRAT)

#if defined(_DIRENT_HAVE_D_RECLEN)
# define reclen(de) ((de)->d_reclen)
#else
# define reclen(de) (sizeof(*(de)) + strlen((de)->d_name))
#endif

static int
scandirat(int dir_fd, const char *dir, struct dirent ***dirlist,
	int (*filter)(const struct dirent *),
	int (*compar)(const struct dirent **, const struct dirent **))
{
	int fd, cnt;
	DIR *dirp;
	struct dirent *de, **ret;

	/* Cannot use O_PATH as we want to use fdopendir() */
	fd = openat(dir_fd, dir, O_RDONLY|O_CLOEXEC);
	if (fd == -1)
		return -1;
	dirp = fdopendir(fd);
	if (!dirp) {
		close(fd);
		return -1;
	}

	ret = NULL;
	cnt = 0;
	while ((de = readdir(dirp))) {
		if (filter(de) == 0)
			continue;

		ret = realloc(ret, sizeof(*ret) * (cnt + 1));
		ret[cnt++] = xmemdup(de, reclen(de));
	}
	*dirlist = ret;

	qsort(ret, cnt, sizeof(*ret), (void *)compar);

	/* closes underlying fd */
	closedir(dirp);

	return cnt;
}

#endif

static void
scandir_free(struct dirent **de, int cnt)
{
	if (cnt <= 0)
		return;

	while (cnt--)
		free(de[cnt]);
	free(de);
}
