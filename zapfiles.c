
/*      zapfiles.c
 *
 *	Copyright 2015 Bob Parker <rlp1938@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *	MA 02110-1301, USA.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <dirent.h>
#include "fileops.h"


static char *helpmsg = "\n\tUsage: zapfiles [option] listfile\n"
  "\tzapfiles deletes the files listed in listfile."
  "\tFiles must be in the format sent by oldfiles to stdout or\n"
  "\tin the format sent to stderr by oldfiles, brokensym or\n "
  "\tduplicates\n"
  "\n\tOptions:\n"
  "\t-h outputs this help message.\n"
  ;
static char* pathend = "!*END*!";

static void dohelp(int forced);

int main(int argc, char **argv)
{
	int opt;
	struct stat sb;
	FILE *fpi;
	char buf[PATH_MAX];
	char *cp;

	while((opt = getopt(argc, argv, ":h")) != -1) {
		switch(opt){
		case 'h':
			dohelp(0);
		break;
		case ':':
			fprintf(stderr, "Option %c requires an argument\n",optopt);
			dohelp(1);
		break;
		case '?':
			fprintf(stderr, "Illegal option: %c\n",optopt);
			dohelp(1);
		break;
		} //switch()
	} //while()
	// now process the non-option arguments

	// 1.Check that argv[???] exists.
	if (!(argv[optind])) {
		fprintf(stderr, "No file provided\n");
		dohelp(1);
	}

	// 2. Check that it's meaningful, ie file/dir exists.
	if (stat(argv[optind], &sb) == -1){
		perror(argv[optind]);
		exit(EXIT_FAILURE);
	}

	// 3. Check that the thing is a file
	if (!S_ISREG(sb.st_mode)) {
		fprintf(stderr, "%s is not a regular file\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

	// open the file and zap them
	fpi = dofopen(argv[optind], "r");
	while((cp = fgets(buf, PATH_MAX, fpi))) {
		char *eod;

		// have to deal with more than one convention here

		// Will not abort on error, there may be non-standard lines that
		// occur from the output to stderr and stdout. Just list them.

		// 1. broken symlinks
		eod = memmem(buf, PATH_MAX, ": No such file",
						strlen(": No such file"));
		if(eod) *eod = '\0';
		// 2. list from oldfiles
		if (!(eod)) {
			eod = memmem(buf, PATH_MAX, pathend, strlen(pathend));
			if (eod) *eod = '\0';
		}
		if (eod) {
			if (unlink(buf) == -1) {
				perror(buf);
			}
		} else {
			fprintf(stderr, "Unknown line in file: %s", buf);
		}
	}

	return 0;
}

void dohelp(int forced)
{
  fputs(helpmsg, stderr);
  exit(forced);
}

