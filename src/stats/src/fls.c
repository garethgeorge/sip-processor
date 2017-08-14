/*MODULE	fls: "Formatted File Listing" "Thu 03 Oct 1985" */
/*PGMR  	fls: "Gary Perlman" "Wang Institute, Tyngsboro, MA 01879 USA" */

#ifdef __STDC__
#include "stdlib.h"
#else
extern	char	*malloc ();
extern	char	*strcpy ();
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define strdup(s) strcpy ((char *) malloc ((unsigned) (strlen (s) + 1)), s)

#include <sys/types.h>
#include <sys/stat.h>

#ifdef unix
#include <pwd.h>
#include <grp.h>
#endif

typedef	int 	Status;
#define	SUCCESS   ((Status) 0)
#define	FAILURE   ((Status) 1)
typedef	int 	Boole;
#define	TRUE      ((Boole) 1)
#define	FALSE     ((Boole) 0)
#define	EOS       '\0'
#define	EOL       '\n'
#define	UNSET     (-1)
typedef	struct	stat	STAT;

Status	fls ();        /* format file listing */
static char	*ugname ();    /* user/group name */
static void	rwxmode ();    /* get rwx mode */
static void	printhelp ();  /* print a menu of the allowed formats */

/*FUNCTION main: main test routine */
#ifdef FLS /* useful program to test main function */

main (argc, argv) char **argv;
	{
	int 	i;                   /* loop through files with this */
	char	buf[BUFSIZ];         /* will hold formatted listing of one file */
	Status	status = SUCCESS;    /* exit status */
	char	*format = argv[1];   /* format string */

	if (argc == 1)
		{
		fprintf (stderr, "Usage: %s format-string [files]\n", argv[0]);
		exit (FAILURE);
		}

	/* check format string by sending NULL file to fls */
	if (fls ((char *) NULL, format, buf) == FAILURE)
		{
		fprintf (stderr, "%s: Bad format string '%s'\n", argv[0], format);
		exit (FAILURE);
		}

	for (i = 2; i < argc; i++)
		{
		if (fls (argv[i], format, buf) == SUCCESS)
			puts (buf);
		else
			{
			fprintf (stderr, "%s: Can't list '%s'\n", argv[0], argv[i]);
			status++;
			}
		}
	exit (status);
	}

#endif /* FLS */

#ifdef	unix

/*FUNCTION ugname: get user or group id string name */
/*NOTE save names and groups because of high cost of access
       not the most efficient method, but adequate for this routine */
#define	MAXSTORE   50             /* store this many group/user id's */
static	char	*Gname[MAXSTORE]; /* group name */
static	int 	Gnum[MAXSTORE];   /* group number */
static	int 	Ngnames = 0;      /* number of group names */
static	char	*Uname[MAXSTORE]; /* user names */
static	int 	Unum[MAXSTORE];   /* user id numbers */
static	int 	Nunames = 0;      /* number of user names */

static
char *
ugname (id, group)  /* return user or group id name */
int 	id;         /* user or group id of a file */
Boole	group;      /* id is a group id, not a user id */
	{
	static	struct	passwd *pw;
	struct	passwd	*getpwuid ();
	static	struct	group *gr;
	struct	group	*getgrgid ();
	int 	i;

	if (group)
		{
		for (i = 0; i < Ngnames; i++)
			if (Gnum[i] == id)
				return (Gname[i]);
		gr = getgrgid (id);
		if (gr == NULL)
			return ("?group");
		if (Ngnames < MAXSTORE)
			{
			Gname[Ngnames] = strdup (gr->gr_name);
			Gnum[Ngnames] = id;
			return (Gname[Ngnames++]);
			}
		else
			return (gr->gr_name);
		}
	else /* User name */
		{
		for (i = 0; i < Nunames; i++)
			if (Unum[i] == id)
				return (Uname[i]);
		pw = getpwuid (id);
		if (pw == NULL)
			return ("?user");
		if (Nunames < MAXSTORE)
			{
			Uname[Nunames] = strdup (pw->pw_name);
			Unum[Nunames] = id;
			return (Uname[Nunames++]);
			}
		else
			return (pw->pw_name);
		}
	}
#endif

/*FUNCTION rwxmode: construct rwx ls style format for file protections */
static
void
rwxmode (stbuf, buf)   /* return rwxrwxrwx type string about file */
STAT 	stbuf;         /* file statistics */
char	*buf;          /* answer pushed in here */
	{
#ifdef	unix
	static	int uid = UNSET;     /* program caller's user id */
	static	int gid = UNSET;     /* program caller's group id */
#endif
	int 	i;

	for (i = 0; i < 9; i += 3)
		{
#ifdef macintosh
		buf[i+0] = '-':
		buf[i+1] = '-':
		buf[i+2] = '-':
#else
		buf[i+0] = (stbuf.st_mode & (S_IREAD  >> i)) ? 'r' : '-';
		buf[i+1] = (stbuf.st_mode & (S_IWRITE >> i)) ? 'w' : '-';
		buf[i+2] = (stbuf.st_mode & (S_IEXEC  >> i)) ? 'x' : '-';
#endif
		}

#ifdef	unix
	/* map user's permissions to upper case (due to Phil Mercurio) */
	if (uid == UNSET)
		uid = getuid ();
	if (gid == UNSET)
		gid = getgid ();
	i = (stbuf.st_uid == uid) ? 0 : (stbuf.st_gid == gid ? 3 : 6);
	if (islower (buf[i+0])) buf[i+0] = toupper (buf[i+0]);
	if (islower (buf[i+1])) buf[i+1] = toupper (buf[i+1]);
	if (islower (buf[i+2])) buf[i+2] = toupper (buf[i+2]);
#endif
	}

/*FUNCTION statstdin: gets stats on stdin */
static
void
statstdin (statptr)
struct	stat	*statptr;
	{
	long 	clock, time ();
	clock = time (0);
	statptr->st_atime = clock;
	statptr->st_mtime = clock;
	statptr->st_ctime = clock;
#ifdef	unix
	statptr->st_gid = getgid ();
	statptr->st_uid = getuid ();
#endif
	statptr->st_ino = 0;
	statptr->st_nlink = 0;
	statptr->st_mode = 0;
	statptr->st_size = 0;
	}

/*FUNCTION fls: formated listing of file */
Status
fls (file, format, bptr) /* put formatted file listing in buffer */
char	*file;           /* name of the file to print */
char	*format;         /* print format control string, like printf */
char	*bptr;           /* pointer to buffer in which to print stats */
	{
	STAT	stbuf;           /* file statistics read in here */
	Boole	sformat;         /* print info in string format */
	char	tformat[BUFSIZ]; /* temporary format string */
	char	*tfptr;          /* pointer to temp format string */
	char	mode[10];        /* file protection stored here */
	char	*filetype;       /* plain, directory, etc */
	extern	char *ctime ();  /* character format of times */
	
	mode[9] = EOS;
	*bptr = EOS;

	if (file == NULL) /* just a test run to check this format string */
		file = "";
	else if (*file == '-' && file[1] == '\0')
		statstdin (&stbuf);
	else if (stat (file, &stbuf))
		return (FAILURE);
	
	while (*format)
		{
		while (*bptr)
			bptr++;

		if (*format != '%')
			{
			*bptr++ = *format++;
			*bptr = EOS;
			}
		else
			{
			/* create a temporary format string and write into buffer */
			*tformat = *format++;
			tfptr = tformat+1;
			while (isdigit (*format) || *format == '-' || *format == '.')
				*tfptr++ = *format++;
			if (*format == EOS) /* bad format string */
				return (FAILURE);
			if (isupper (*format)) /* upper case formats are all strings */
				{
				sformat = TRUE;
				*tfptr++ = 's';
				}
			else
				{
				sformat = FALSE;
				*tfptr++ = 'd';
				}
			*tfptr = EOS;
			switch (*format++)
				{
				default:
					return (FAILURE);
					/* NOTREACHED */

				case '?': /*FORM puts ("?	print this list of formats"); */
					printhelp ();
					break;

				case '%': /*FORM puts ("%	insert %"); */
					*bptr++ = '%';
					*bptr = EOS;
					break;

				case 'A': case 'a': /*FORM puts ("aA	access time"); */
					if (sformat)
						(void) sprintf (bptr, tformat, ctime (&stbuf.st_atime));
					else
						(void) sprintf (bptr, tformat, stbuf.st_atime);
					break;

				case 'C': case 'c': /*FORM puts ("cC	change time"); */
					if (sformat)
						(void) sprintf (bptr, tformat, ctime (&stbuf.st_ctime));
					else
						(void) sprintf (bptr, tformat, stbuf.st_ctime);
					break;
#ifdef unix
				case 'G': case 'g': /*FORM puts ("gG	group id"); */
					if (sformat == FALSE)
						(void) sprintf (bptr, tformat, stbuf.st_gid);
					else
						(void) sprintf (bptr, tformat, ugname (stbuf.st_gid, TRUE));
					break;
#endif
				case 'i': /*FORM puts ("i	inode number"); */
					(void) sprintf (bptr, tformat, stbuf.st_ino);
					break;

				case 'l': /*FORM puts ("l	number of links"); */
					(void) sprintf (bptr, tformat, stbuf.st_nlink);
					break;

				case 'M': case 'm': /*FORM puts ("mM	modification time"); */
					if (sformat)
						(void) sprintf (bptr, tformat, ctime (&stbuf.st_mtime));
					else
						(void) sprintf (bptr, tformat, stbuf.st_mtime);
					break;

				case 'n': /*FORM puts ("n	insert a newline"); */
					*bptr++ = EOL;
					*bptr = EOS;
					break;

				case 'N': /*FORM puts ("N	file name"); */
					(void) sprintf (bptr, tformat, file);
					break;

				case 'P': case 'p': /*FORM puts ("pP	protection modes"); */
					if (sformat == FALSE)
						{
						*(tfptr-1) = 'o';
						(void) sprintf (bptr, tformat, (stbuf.st_mode & 0777));
						}
					else
						{
						rwxmode (stbuf, mode);
						(void) sprintf (bptr, tformat, mode);
						}
					break;

				case 's': /*FORM puts ("s	size"); */
					(void) sprintf (bptr, tformat, stbuf.st_size);
					break;

				case 't': /*FORM puts ("t	insert a tab"); */
					*bptr++ = '\t';
					*bptr = EOS;
					break;

				case 'T': /*FORM puts ("T	file type"); */
					switch (stbuf.st_mode & S_IFMT)
						{
						default: filetype = "?unknown";               break;
						case S_IFDIR: filetype = "directory";         break;
						case S_IFCHR: filetype = "character special"; break;
						case S_IFBLK: filetype = "block special";     break;
						case S_IFREG: filetype = "-plain";            break;
#					ifdef S_IFLNK
						case  S_IFLNK: filetype = "symbolic link";    break;
#					endif /* S_IFLNK */
#					ifdef S_IFSOCK
						case  S_IFSOCK: filetype = "Socket";          break;
#					endif /* S_IFSOCK */
						}
					(void) sprintf (bptr, tformat, filetype);
					break;
#ifdef unix
				case 'U': case 'u': /*FORM puts ("uU	user id"); */
					if (sformat == FALSE)
						(void) sprintf (bptr, tformat, stbuf.st_uid);
					else
						(void) sprintf (bptr, tformat, ugname (stbuf.st_uid, FALSE));
					break;
#endif
				}
			}
		}
	return (SUCCESS);
	}

/*FUNCTION printhelp: print help message about formats */
static
void
printhelp ()
	{
	puts ("Summary of % formats:");
	puts ("xX	means that both integer (x) & string (X) formats supported");
	puts ("All formats support %-p.wX where:");
	puts ("	- is an optional sign to left-justify X");
	puts ("	p is an optional pad of white spaces");
	puts ("	w is an optional maximum width of X");
	puts ("Char	Meaning:");
	/* the rest is constructed with seec -e -t "FORM " % */
	puts ("?	print this list of formats"); 
	puts ("%	insert %");
	puts ("aA	access time"); 
	puts ("cC	change time"); 
	puts ("gG	group id"); 
	puts ("i	inode number"); 
	puts ("l	number of links"); 
	puts ("mM	modification time"); 
	puts ("n	insert a newline"); 
	puts ("N	file name"); 
	puts ("pP	protection modes"); 
	puts ("s	size"); 
	puts ("t	insert a tab"); 
	puts ("T	file type"); 
	puts ("uU	user id"); 
	}
