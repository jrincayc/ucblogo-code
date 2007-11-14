
// Copyright Timothy Miller, 1999

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <grp.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <pwd.h>
#include <utmp.h>
#include <stdlib.h>

int master_fd;

static char pty_name[12];
static int uid, gid;

void remove_utmp();
void add_utmp(int);

int pty_master()
{
	int i, mfd;
	char *ptr;
	struct stat statbuf;
	static char ptychar[] = "pqrstuvwxyzPQRST";
	static char hexdigit[] = "0123456789abcdef";

	for (ptr = ptychar; *ptr; ptr++) {
		strcpy(pty_name, "/dev/ptyXY");
		pty_name[8] = *ptr;
		pty_name[9] = '0';

		if (stat(pty_name, &statbuf) < 0) break;

		for (i=0; i<16; i++) {
			pty_name[9] = hexdigit[i];
			mfd = open(pty_name, O_RDWR);
			if (mfd >= 0) return mfd;
		}
	}
	return -1;
}

int pty_slave()
{
	int i, sfd;
	struct group *gptr;
	int gid;

	if ((gptr = getgrnam("tty")) != 0) {
		gid = gptr->gr_gid;
	} else {
		gid = -1;
	}

	chown(pty_name, getuid(), gid);
	chmod(pty_name, S_IRUSR|S_IWUSR|S_IWGRP);

	sfd = open(pty_name, O_RDWR);
	return sfd;
}

struct stat tty_stat;

int spawn(char *exe)
{
	int mfd, pid, sfd, fd, i;
	int uid, gid;

	uid = getuid();
	gid = getgid();

	mfd = pty_master();

	if (mfd < 0) {
		fprintf(stderr, "Can't open master\n");
		return -1;
	}

        pty_name[5] = 't';
	stat(pty_name, &tty_stat);

	pid = fork();
	if (pid<0) {
		fprintf(stderr, "Can't fork\n");
		return -1;
	}

	if (!pid) {
		close(mfd);

		sfd = pty_slave();
		if (sfd<0) {
			fprintf(stderr, "Can't open child\n");
			return -1;
		}

		setuid(uid);
		setgid(gid);

		if (setsid()<0) 
			fprintf(stderr, "Could not set session leader\n");
		if (ioctl(sfd, TIOCSCTTY, NULL)) 
			fprintf(stderr, "Could not set controllint tty\n");

		dup2(sfd, 0);
		dup2(sfd, 1);
		dup2(sfd, 2);
		if (sfd>2) close(sfd);

		execl(exe, exe, NULL);
		exit(0);
	}

	add_utmp(pid);

	master_fd = mfd;
	return mfd;
}

void restore_ttyp()
{
	chown(pty_name, tty_stat.st_uid, tty_stat.st_gid);
	chmod(pty_name, tty_stat.st_mode);

	remove_utmp();
}

struct utmp ut_entry;

void add_utmp(int spid)
{

	ut_entry.ut_type = USER_PROCESS;
	ut_entry.ut_pid = spid;
	strcpy(ut_entry.ut_line, pty_name+5);
	strcpy(ut_entry.ut_id, pty_name+8);
	time(&ut_entry.ut_time);
	strcpy(ut_entry.ut_user, getpwuid(getuid())->pw_name);
	strcpy(ut_entry.ut_host, getenv("DISPLAY"));
	ut_entry.ut_addr = 0;
	setutent();
	pututline(&ut_entry);
	endutent();
}

void remove_utmp()
{
	ut_entry.ut_type = DEAD_PROCESS;
	memset(ut_entry.ut_line, 0, UT_LINESIZE);
	ut_entry.ut_time = 0;
	memset(ut_entry.ut_user, 0, UT_NAMESIZE);
	setutent();
	pututline(&ut_entry);
	endutent();
}

