/*************************************************************************

$Id$

Copyright (c) 2000-2006, webbie@ipfw.org
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

Neither name of the Webbie nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*************************************************************************/


#define FNAMELEN		250
#define MAXSFVFILE		1024
#define BSDSFV_VERSION	"BSDSFV for UNiX, $Revision: 1.18 $"
#define MISSINGTAG 		".missing"
#define BADTAG 			".bad"

typedef struct sfvtable {
	char filename[FNAMELEN];
	int crc;
	int found;
} SFVTABLE;


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>


long
UpdateCRC(unsigned long CRC, const char *buffer, long count)
{
	/*
	 * Note: if you want to know how CRC32-checking works, I
	 * recommend grabbing any old (mid 90's) Z-Modem source.
	 * There is not much you can change in this function, so
	 * if you need a CRC32-check yourself, feel free to rip.
	 */
	unsigned long CRCTABLE[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
		    0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e,
		    0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064,
		    0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551,
		    0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f,
		    0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
		    0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa,
		    0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf,
		    0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5,
		    0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2,
		    0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
		    0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5,
		    0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb,
		    0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8,
		    0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6,
		    0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
		    0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541,
		    0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846,
		    0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c,
		    0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409,
		    0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
		    0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c,
		    0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12,
		    0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27,
		    0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d,
		    0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
		    0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8,
		    0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd,
		    0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3,
		    0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0,
		    0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
		    0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b,
		    0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9,
		    0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae,
		    0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4,
		    0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
		    0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff,
		    0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354,
		    0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a,
		    0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f,
		    0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
		    0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02,
		    0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	if (buffer && count) {
		do {
			CRC =
			    ((CRC >> 8) & 0xFFFFFF) ^
			    CRCTABLE[(unsigned char)((CRC & 0xff) ^
				*buffer++)];
		}
		while (--count);

	}
	return CRC;
}

/* ^ UpdateCRC ^ */



long
GetFileCRC(char *filename)
{
	unsigned long crc = 0xffffffff;
	FILE *f;
	long totalread = 0;
	long localread;

	/*
	 * Note: different buffer sizes may result in noticable 
	 * different performance depending on system, so feel
	 * free to modify.
	 */
#define BUFFERSIZE 65536*16
	char buffer[BUFFERSIZE];

	if ((f = fopen(filename, "rb")) != NULL) {
		do {
			if ((localread = fread(buffer, 1, BUFFERSIZE, f))) {
				crc = UpdateCRC(crc, buffer, localread);
				totalread = totalread + localread;
			}
		}
		while (localread > 0);
		fclose(f);

		crc = crc ^ 0xffffffff;
	} else {		/* error opening file */

		fprintf(stderr, "\nFatal error: cannot read file: %s\n",
		    filename);
	}

	return crc;
}

int
FileExists(char *name)
{
	FILE *testfile;

	testfile = fopen(name, "rb");
	if (testfile != NULL) {
		fclose(testfile);
		return 1;
	}
	return 0;
}


int
CheckFileExists(char *name, SFVTABLE sfvTable[], int n)
{
	while (n > 0) {
		if (!strncasecmp(sfvTable[--n].filename, name, FNAMELEN))
			return n;
	}
	return (-1);
}


void
usage(char *prog)
{
	printf("\nUsage: %s mode [option] sfv-filename [filename...]\n\n",
	    prog);
	printf
	    ("Possible modes: -c : create SFV file (only read .rar file as default, see -a)\n");
	printf
	    ("                -t : test single file(s) (multiple filenames possible)\n");
	printf("                -T : test whole SFV file\n");
	printf("                -m : count missing files\n");
	printf
	    ("Options:        -l <filename> : use <filename> as banner file (create mode)\n");
	printf
	    ("                -w : Win-SFV emulation mode                   (create mode)\n");
	printf
	    ("                -a : add all files to .sfv (even .nfo etc.)   (create mode)\n");
	printf
	    ("                -d : add files' dates & times to SFV file     (create mode)\n");
	printf
	    ("                -g : glftpd mode     (counting and test whole mode)\n\n");
	printf("Examples:\n");
	printf
	    ("(1) Create test.sfv and add test.* to it       : %s -c test.sfv test.*\n",
	    prog);
	printf
	    ("(2) Check file test.r02 against CRC in test.sfv: %s -t test.sfv test.r02\n",
	    prog);
	printf
	    ("(3) Check all files listed in test.sfv         : %s -T test.sfv\n",
	    prog);
	printf
	    ("(4) Count number of files missing of test.sfv  : %s -m test.sfv\n",
	    prog);
	printf
	    ("(5) Create myrls.sfv with myrls.* in it, use Win-SFV compatibility mode and file\n");
	printf
	    ("/tmp/mylogo.nfo as banner: %s -c -w -l /tmp/mylogo.nfo myrls.sfv myrls.*\n",
	    prog);
	printf("\n");
	printf
	    ("Read documentation files for more help and check for updates regularily!\n\n");
	return;
}


int
main(int argc, char *argv[])
{
	int compatmode = 0;	// Win-SFV compatibility mode
	int haslogo = 0;	// add banner to created SFVs
	int glftpdmode = 0;	// glftpd mode
	int doallfiles = 0;	// include even crappy files to SFV
	int addfinfo = 0;	// add files' dates & times to SFV
	char logoname[FNAMELEN];	// filename of bannerfile
	char sfvname[FNAMELEN];	// name of .sfv file to use
	char sfvline[FNAMELEN];	// current line in SFV file
	int mode = 0;		// mode of operation             
	int paramcnt = 0;	// command line parameter offset

	SFVTABLE sfvTable[MAXSFVFILE];
	char COMPLETETAG[] = "[000\%]-[-all.files.CRC.ok-]";
	char OLDTAG[FNAMELEN];
	char NEWTAG[FNAMELEN];

	FILE *sfvfile;
	FILE *logofile;
	FILE *missingfile;
	DIR *dirp;
	struct dirent *dp;
	long mycrc;
	int cnt;
	int dothisone;
	char cfname[FNAMELEN];
	char crap[FNAMELEN];
	char precent[5];
	int numfiles = 0;
	int badfiles = 0;
	int msgfiles = 0;
	int missingfiles = 0;
	int paramsave;
	struct stat fileinfo;
	time_t curtime;
	struct tm zeit;
	char mytime[80];
	char ch;

// output program name & version number
	printf("%s\n", BSDSFV_VERSION);

// parse command line
	cnt = 0;
	while ((ch = getopt(argc, argv, "acdgmtTvwl:")) != -1) {
		switch (ch) {
		case 'c':	// prepare for create mode
			mode = 1;
			cnt++;
			break;

		case 't':	// prepare single file test mode
			mode = 2;
			cnt++;
			break;

		case 'T':	// prepare sfv file test mode
			mode = 3;
			cnt++;
			break;

		case 'm':	// prepare missing file test mode
			mode = 4;
			cnt++;
			break;

		case 'v':	// print version and exit
			exit(0);
			break;

		case 'l':	// set banner file name
			haslogo = 1;
			sprintf(logoname, "%s", optarg);
			break;

		case 'a':	// set lame allfiles-add-mode
			doallfiles = 1;
			break;

		case 'w':	// set win-sfv compatibility mode
			compatmode = 1;
			break;

		case 'd':	// add files' dates & times to SFV
			addfinfo = 1;
			break;

		case 'g':	// set glftpd mode
			glftpdmode = 1;
			break;

		case '?':
		default:
			usage(argv[0]);
			return 1;
			break;
		}
	}

	if (argc == optind || cnt != 1) {
		usage(argv[0]);
		return 1;
	}
	argc -= optind;
	argv += optind;

	strncpy(sfvname, argv[0], FNAMELEN);
	argc--;
	argv++;

	if (mode == 1) {
		printf("\nCreating new checksum file: %s\n", sfvname);

		if (haslogo)
			printf("Adding banner file        : %s\n", logoname);

		if (compatmode)
			printf("Using compatibility mode  : Win-SFV\n");

		if (doallfiles)
			printf("Add-all-files mode active\n");

		if (addfinfo)
			printf("Adding files' times & dates to SFV file\n");

		printf("\n");

		sfvfile = fopen(sfvname, "wt");
		if (sfvfile == NULL) {
			fprintf
			    (stderr,
			    "Oh mama! We can\'t write to %s (check permissions!), aborting ...\n",
			    sfvname);
			return 1;
		}

		time(&curtime);
		zeit = *localtime(&curtime);
		strftime(mytime, 50, "%Y-%m-%d at %H:%M.%S", &zeit);

		if (compatmode)
			fprintf(sfvfile,
			    "; Generated by WIN-SFV32 v1.1a on %s\r\n;\r\n",
			    mytime);
		else
			fprintf(sfvfile, "; Generated by %s on %s\r\n;\r\n",
			    BSDSFV_VERSION, mytime);

		if (haslogo) {
			logofile = fopen(logoname, "rt");
			if (logofile == NULL) {
				fprintf
				    (stderr,
				    "Oh mama! I couldn\'t find logo file %s ... omitting!\n",
				    logoname);
			} else {
				while (fgets(sfvline, FNAMELEN,
					logofile) != NULL) {
					for (cnt = 0; cnt < strlen(sfvline);
					    cnt++)
						if (sfvline[cnt] == '\n')
							sfvline[cnt] = '\0';
					fprintf(sfvfile, "; %s\r\n", sfvline);
				}
				fclose(logofile);
			}
		}		// add logo

		if (addfinfo) {
			paramsave = paramcnt;
			while (paramsave < argc) {
				stat(argv[paramsave], &fileinfo);

				zeit = *localtime(&fileinfo.st_mtime);
				strftime(mytime, 50, "%H:%M.%S %Y-%m-%d",
				    &zeit);

				fprintf(sfvfile, "; %12d  %s %s\r\n",
				    (int)fileinfo.st_size, mytime,
				    argv[paramsave]);

				paramsave++;
			}
			fprintf(sfvfile, ";\r\n");
		}


		while (paramcnt < argc) {
			sprintf(cfname, "%s", argv[paramcnt]);

			// check if we should process this file
			dothisone = 1;
			if (!doallfiles) {
				if (strstr(cfname, ".r") == NULL &&
				    strstr(cfname, ".R") == NULL &&
				    strstr(cfname, ".0") == NULL)
					dothisone = 0;
			}

			if (dothisone) {
				printf("Adding file: %s ... ", cfname);
				fflush(stdout);
				mycrc = GetFileCRC(cfname);
				printf("CRC = 0x%08lX\n", mycrc);
				sprintf(sfvline, "%s %08lX", cfname, mycrc);

				// uncomment next 2 lines to 
				// convert filename to upper case, for whatever reason
				// for (cnt = 0; sfvline[cnt] != '\0'; cnt++)
				//   sfvline[cnt] = toupper (sfvline[cnt]);

				fprintf(sfvfile, "%s\r\n", sfvline);
				numfiles++;
			}
			paramcnt++;
		}

		fclose(sfvfile);

		printf
		    ("\nSFV file successfully created (%d files processed) ...\n\n",
		    numfiles);

	}			// create SFV file mode


	if (mode == 2) {
		while (paramcnt < argc) {
			sprintf(cfname, "%s", argv[paramcnt]);
			printf("Testing %s ... ", cfname);
			fflush(stdout);
			mycrc = GetFileCRC(cfname);
			printf("local = 0x%08lX, listed = ", mycrc);
			fflush(stdout);

			sfvfile = fopen(sfvname, "rt");
			if (sfvfile == NULL) {
				fprintf(stderr,
				    "Oh mama! I can\'t open %s ... :-(\n",
				    sfvname);
				return 1;
			}

			dothisone = 0;

			if (mycrc != 0xffffffff) {
				while (fgets(sfvline, FNAMELEN,
					sfvfile) != NULL) {
					// Skip comments, blank lines, other crap
					if (sfvline[0] != ';'
					    && sfvline[0] != '#'
					    && sfvline[0] != ' '
					    && strlen(sfvline) > 8) {
						for (cnt = strlen(sfvline); cnt >= 0 && sfvline[cnt] != ' '; cnt--) ;
						if (cnt > 0 ) {
							strncpy(sfvTable[0].filename, sfvline, cnt);
							sscanf(sfvline+cnt+1, "%X", &sfvTable[0].crc);
						}

						if (strncasecmp(cfname,
							sfvTable[0].filename,
							strlen(cfname)) == 0) {
							dothisone = 1;
							sfvTable[0].found = 1;
							break;
						}
					}
				}
			}

			if (dothisone) {
				printf("0x%08X - ", sfvTable[0].crc);

				if (sfvTable[0].crc == mycrc) {
					printf("OK\n");
					if (glftpdmode) {
						sprintf(crap, "%s%s",
						    sfvTable[0].filename,
						    MISSINGTAG);
						unlink(crap);
						sprintf(crap, "%s%s",
						    sfvTable[0].filename,
						    BADTAG);
						unlink(crap);
					}
				} else {
					printf("BAD\n");
					badfiles++;
					if (glftpdmode) {
						sprintf(crap, "%s%s",
						    sfvTable[0].filename,
						    MISSINGTAG);
						unlink(crap);
						sprintf(crap, "%s%s",
						    sfvTable[0].filename,
						    BADTAG);
						rename(cfname, crap);
					}
				}
			} else {
				printf("MISSING\n");
				missingfiles++;
			}
			numfiles++;
			paramcnt++;
			fclose(sfvfile);
		}

		printf
		    ("\n%d file(s) tested - %d OK - %d bad - %d missing...\n",
		    numfiles, numfiles - badfiles - missingfiles, badfiles,
		    missingfiles);
		fflush(stdout);


		if (badfiles || missingfiles)
			return 1;
		return 0;
	}			// test single file(s) mode


	if (mode == 3) {
		printf("\nProcessing complete check of %s ...\n", sfvname);

		if (glftpdmode)
			printf("Using glftpd mode\n");

		/*
		 * Read the whole sfv file into memory 
		 */
		sfvfile = fopen(sfvname, "rt");
		if (sfvfile == NULL) {
			fprintf(stderr, "Oh mama! I can\'t open %s ... :-(\n",
			    sfvname);
			return 1;
		}
		numfiles = 0;
		while (fgets(sfvline, FNAMELEN, sfvfile) != NULL) {
			// Skip comments, blank lines, other crap
			if (sfvline[0] != ';' && sfvline[0] != '#'
			    && sfvline[0] != ' ' && strlen(sfvline) > 8) {
				sfvTable[cnt].found = 0;

				for (cnt = strlen(sfvline); cnt >= 0 && sfvline[cnt] != ' '; cnt--) ;
				if (cnt > 0 ) {
					strncpy(sfvTable[numfiles].filename, sfvline, cnt);
					sscanf(sfvline+cnt+1, "%X", &sfvTable[numfiles].crc);
				}

				numfiles++;
				if (numfiles >= MAXSFVFILE) {
					printf
					    ("bsdsfv cannot handle more than %d files\n",
					    MAXSFVFILE);
					return 1;
				}
			}
		};
		fclose(sfvfile);

		/*
		 * Read current directory file by file 
		 */
		missingfiles = 0;
		badfiles = 0;
		OLDTAG[0] = '\0';
		dirp = opendir(".");
		while ((dp = readdir(dirp)) != NULL) {
			if (glftpdmode &&
			    (dp->d_type == DT_DIR || dp->d_type == DT_UNKNOWN)
			    && (strlen(dp->d_name) == strlen(COMPLETETAG))
			    && (dp->d_name[0] == '[')
			    && !strncmp(dp->d_name + 5, COMPLETETAG + 5,
				strlen(COMPLETETAG) - 5)) {
				strcpy(OLDTAG, dp->d_name);
			}
			if (dp->d_type == DT_REG || dp->d_type == DT_UNKNOWN) {
				cnt =
				    CheckFileExists(dp->d_name, sfvTable,
				    numfiles);
				if (cnt >= 0) {
					sfvTable[cnt].found = 1;

					printf
					    ("Testing %s ... listed = 0x%08X ... ",
					    dp->d_name, sfvTable[cnt].crc);

					fflush(stdout);

					mycrc = GetFileCRC(dp->d_name);

					printf("local = 0x%08lX ... ", mycrc);

					if (mycrc == sfvTable[cnt].crc) {
						printf("OK\n");
					} else {
						if (mycrc == 0xffffffff) {
							printf("MISSING\n");
							missingfiles++;
						} else {
							printf("BAD\n");
							badfiles++;
						}
					}
					fflush(stdout);
				}
			}
		}
		(void)closedir(dirp);

		for (cnt = 0; cnt < numfiles; cnt++) {
			if (sfvTable[cnt].found) {
				if (glftpdmode) {
					sprintf(crap, "%s%s",
					    sfvTable[cnt].filename,
					    MISSINGTAG);
					unlink(crap);
				}
			} else {
				printf
				    ("Testing %s ... listed = 0x%08X ... Local = MISSING\n",
				    sfvTable[cnt].filename, sfvTable[cnt].crc);
				missingfiles++;
				if (glftpdmode) {
					sprintf(crap, "%s%s",
					    sfvTable[cnt].filename,
					    MISSINGTAG);
					missingfile = fopen(crap, "w+");
					if (missingfile != NULL) {
						fclose(missingfile);
					}
				}
			}
		}

		if (glftpdmode) {
			if (numfiles > 0) {
				cnt =
				    ((numfiles -
					missingfiles) * 100) / numfiles;
				if (cnt > 100)
					cnt = 100;
			} else {
				cnt = 0;
			}

			sprintf(precent, "%.3d", cnt);
			strcpy(NEWTAG, COMPLETETAG);
			NEWTAG[1] = precent[0];
			NEWTAG[2] = precent[1];
			NEWTAG[3] = precent[2];

#ifdef DEBUG
			printf("cnt %d OLD %s NEW %s\n", cnt, OLDTAG, NEWTAG);
#endif

			if (strcmp(OLDTAG, NEWTAG)) {
				rmdir(OLDTAG);
				if (mkdir(NEWTAG,
					S_IRWXU | S_IRWXG | S_IRWXO) == 0)
					chmod(NEWTAG,
					    S_IRWXU | S_IRWXG | S_IRWXO);
			}
			printf("Completion Status: %s\n", NEWTAG);
		}

		printf
		    ("\n%d file(s) tested - %d OK - %d bad - %d missing ...\n\n",
		    numfiles, numfiles - (badfiles + missingfiles), badfiles,
		    missingfiles);

		if (missingfiles)
			return 2;

		if (badfiles)
			return 1;

		return 0;
	}

	/*
	 * test whole SFV file mode 
	 */
	if (mode == 4) {
		printf("\nPerforming completion check...\n");

		if (glftpdmode)
			printf("Using glftpd mode\n");

		/*
		 * Read the whole sfv file into memory 
		 */
		sfvfile = fopen(sfvname, "rt");
		if (sfvfile == NULL) {
			fprintf(stderr, "Oh mama! I can\'t open %s ... :-(\n",
			    sfvname);
			return 1;
		}
		numfiles = 0;
		while (fgets(sfvline, FNAMELEN, sfvfile) != NULL) {
			// Skip comments, blank lines, other crap
			if (sfvline[0] != ';' && sfvline[0] != '#'
			    && sfvline[0] != ' ' && strlen(sfvline) > 8) {
				sfvTable[cnt].found = 0;
				sscanf(sfvline, "%s %X",
				    sfvTable[numfiles].filename,
				    &sfvTable[numfiles].crc);
				numfiles++;
				if (numfiles >= MAXSFVFILE) {
					printf
					    ("bsdsfv cannot handle more than %d files\n",
					    MAXSFVFILE);
					return 1;
				}
			}
		};
		fclose(sfvfile);

		/*
		 * Read current directory file by file 
		 */
		OLDTAG[0] = '\0';
		dirp = opendir(".");
		while ((dp = readdir(dirp)) != NULL) {
			if (glftpdmode &&
			    (dp->d_type == DT_DIR || dp->d_type == DT_UNKNOWN)
			    && (strlen(dp->d_name) == strlen(COMPLETETAG))
			    && (dp->d_name[0] == '[')
			    && !strncmp(dp->d_name + 5, COMPLETETAG + 5,
				strlen(COMPLETETAG) - 5)) {
				strcpy(OLDTAG, dp->d_name);
			}
			if (dp->d_type == DT_REG || dp->d_type == DT_UNKNOWN) {
				cnt =
				    CheckFileExists(dp->d_name, sfvTable,
				    numfiles);
				if (cnt >= 0) {
					sfvTable[cnt].found = 1;
				}
			}
		}
		(void)closedir(dirp);

		/*
		 * Read the sfv array and create/delete missing tag file as required 
		 */
		for (cnt = 0; cnt < numfiles; cnt++) {
			if (sfvTable[cnt].found) {
				msgfiles++;
				if (glftpdmode) {
					sprintf(crap, "%s%s",
					    sfvTable[cnt].filename,
					    MISSINGTAG);
					unlink(crap);
				}
			} else {
				if (glftpdmode) {
					sprintf(crap, "%s%s",
					    sfvTable[cnt].filename,
					    MISSINGTAG);
					missingfile = fopen(crap, "w+");
					if (missingfile != NULL) {
						fclose(missingfile);
					}
				}
			}
		}

/*
 Make progress directory
 */
		if (glftpdmode) {
			if (numfiles > 0) {
				cnt = (msgfiles * 100) / numfiles;
				if (cnt > 100)
					cnt = 100;
			} else
				cnt = 0;

			sprintf(precent, "%.3d", cnt);
			strcpy(NEWTAG, COMPLETETAG);
			NEWTAG[1] = precent[0];
			NEWTAG[2] = precent[1];
			NEWTAG[3] = precent[2];

#ifdef DEBUG
			printf("cnt %d OLD %s NEW %s\n", cnt, OLDTAG, NEWTAG);
#endif

			if (strcmp(OLDTAG, NEWTAG)) {
				rmdir(OLDTAG);
				if (mkdir(NEWTAG,
					S_IRWXU | S_IRWXG | S_IRWXO) == 0)
					chmod(NEWTAG,
					    S_IRWXU | S_IRWXG | S_IRWXO);
			}
			printf("Completion Status: %s\n", NEWTAG);
		} else {
			printf
			    ("[total files listed] [local files] [missing files]\n%-4d                 %-4d          %-4d\n",
			    numfiles, msgfiles, numfiles - msgfiles);
		}
		return 0;
	}			/* count files mode */
	return 0;
}
