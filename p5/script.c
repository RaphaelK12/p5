/* Copyright 2008 wParam, licensed under the GPL */

#include <windows.h>
#include "p5.h"
#include <stdio.h>
#include "..\common\wp_parse.h"

#define SCRIPT_FOPEN_FAILED				DERRCODE (SCRIPT_DERR_BASE, 0x01)
#define SCRIPT_TOKENIZE_FAILED			DERRCODE (SCRIPT_DERR_BASE, 0x02)
#define SCRIPT_INSANE_HASH_DETECT		DERRCODE (SCRIPT_DERR_BASE, 0x03)
#define SCRIPT_ABORTED_BY_HASH			DERRCODE (SCRIPT_DERR_BASE, 0x04)
#define SCRIPT_BASE_NESTING_ERROR		DERRCODE (SCRIPT_DERR_BASE, 0x05)
#define SCRIPT_INVALID_LABEL			DERRCODE (SCRIPT_DERR_BASE, 0x06)
#define SCRIPT_INVALID_SKIP_LABEL		DERRCODE (SCRIPT_DERR_BASE, 0x07)
#define SCRIPT_INVALID_HASH_LINE		DERRCODE (SCRIPT_DERR_BASE, 0x08)
#define SCRIPT_INVALID_COUNT_LINE		DERRCODE (SCRIPT_DERR_BASE, 0x09)
#define SCRIPT_INSUFFICIENT_PARAMETERS	DERRCODE (SCRIPT_DERR_BASE, 0x0A)
#define SCRIPT_ABORTED_DUE_TO_ERROR		DERRCODE (SCRIPT_DERR_BASE, 0x0B)
#define SCRIPT_LABEL_NOT_FOUND			DERRCODE (SCRIPT_DERR_BASE, 0x0C)
#define SCRIPT_FILE_NOT_EOF				DERRCODE (SCRIPT_DERR_BASE, 0x0D)
#define SCRIPT_FILE_ERROR				DERRCODE (SCRIPT_DERR_BASE, 0x0E)
#define SCRIPT_UNTERMINATED_COMMAND		DERRCODE (SCRIPT_DERR_BASE, 0x0F)
#define SCRIPT_WTF						DERRCODE (SCRIPT_DERR_BASE, 0x10)
#define SCRIPT_FSEEK_FAILED				DERRCODE (SCRIPT_DERR_BASE, 0x11)
#define SCRIPT_INFINITE_LOOP			DERRCODE (SCRIPT_DERR_BASE, 0x12)
#define SCRIPT_INVALID_GOTOCOUNT_LINE	DERRCODE (SCRIPT_DERR_BASE, 0x13)



char *ScrErrors[] = 
{
	NULL,
	"SCRIPT_FOPEN_FAILED",
	"SCRIPT_TOKENIZE_FAILED",			
	"SCRIPT_INSANE_HASH_DETECT",		
	"SCRIPT_ABORTED_BY_HASH",			
	"SCRIPT_BASE_NESTING_ERROR",		
	"SCRIPT_INVALID_LABEL",			
	"SCRIPT_INVALID_SKIP_LABEL",		
	"SCRIPT_INVALID_HASH_LINE",		
	"SCRIPT_INVALID_COUNT_LINE",		
	"SCRIPT_INSUFFICIENT_PARAMETERS",	
	"SCRIPT_ABORTED_DUE_TO_ERROR",		
	"SCRIPT_LABEL_NOT_FOUND",			
	"SCRIPT_FILE_NOT_EOF",				
	"SCRIPT_FILE_ERROR",				
	"SCRIPT_UNTERMINATED_COMMAND",		
	"SCRIPT_WTF",
	"SCRIPT_FSEEK_FAILED",
	"SCRIPT_INFINITE_LOOP",
	"SCRIPT_INVALID_GOTOCOUNT_LINE",
};

char *ScrGetErrorString (int errindex)
{
	int numerrors = sizeof (ScrErrors) / sizeof (char *);

	if (errindex >= numerrors || !ScrErrors[errindex])
		return "SCRIPT_UNKNOWN_ERROR_CODE";

	return ScrErrors[errindex];
}

int ScrIsHashLine (char *line)
{
	//pass any space or tab characters, then see if it matches

	while (*line && (*line == ' ' || *line == '\t'))
		line++;

	//*line may be "", but that's ok.
	if (strncmp (line, "(#", 2) == 0)
		return 1;
	return 0;
}


#if 0
void ScrHelpWindow (void)
{
	char buffer[4096];

	buffer[0] = '\0';
	buffer[4095] = (char)0xED;

	strcat (buffer, "Script file quick reference:\r\n\r\n");
	strcat (buffer, "//comment line\r\n");
	strcat (buffer, "    Single line comment\r\n");
	strcat (buffer, "$1, $2, $3, etc\r\n");
	strcat (buffer, "    These atoms are automatically bound to the parameters passed to the script.  $0 is bound to the filename passed to the script\r\n");
	strcat (buffer, "    execution function.  In theory it should always be possible to pass $0 to the (script) function to have a script call itself.\r\n");
	strcat (buffer, "(# comment.in)\r\n");
	strcat (buffer, "    Begins a multi-line comment block.  ALL (#) commands are processed, even inside of comments.  If you need to comment out a large\r\n");
	strcat (buffer, "    block of a file, the recommended way to do this is to define a label at the end and then (# skip) to it.\r\n");
	strcat (buffer, "(# comment.out)\r\n");
	strcat (buffer, "    Ends a multi-line comment block\r\n");
	strcat (buffer, "(# error.abort)\r\n");
	strcat (buffer, "    Sets error mode to 'abort'.  Any script line that results in an error will cause script processing to stop immediately.\r\n");
	strcat (buffer, "    This mode is the default\r\n");
	strcat (buffer, "(# error.cont)\r\n");
	strcat (buffer, "    Sets error mode to 'continue'.  All errors generated by script lines will be ignored, and script processing will continue.\r\n");
	strcat (buffer, "(# error.vcont)\r\n");
	strcat (buffer, "    Sets error mode to 'verbose continue'.  All errors generated will be printed to the console, if one is open, and script\r\n");
	strcat (buffer, "    processing will continue.\r\n");
	strcat (buffer, "(# error.skip [label])\r\n");
	strcat (buffer, "    Sets error mode to 'skip', meaning that if an error occurs, the script will advance to the given label and then continue as\r\n");
	strcat (buffer, "    normal.  The atom \'lasterror\' will store the error string returned, or \"\" if no error occured.\r\n");
	strcat (buffer, "(# error.vskip [label])\r\n");
	strcat (buffer, "    Same as error.skip, but also prints errors to the console\r\n");
	strcat (buffer, "(# label [label])\r\n");
	strcat (buffer, "    Makes the line a script label.  Labels are limited to 255 characters.\r\n");
	strcat (buffer, "(# return)\r\n");
	strcat (buffer, "    Ends script processing successfully.\r\n");
	strcat (buffer, "(# abort [message])\r\n");
	strcat (buffer, "    Ends script processing immediately with a failure status.  The message is returned to the caller.\r\n");
	strcat (buffer, "(# base)\r\n");
	strcat (buffer, "    Causes the script to fail if there is any expression pending.  Use this to make sure that a long, complex s-expression is\r\n");
	strcat (buffer, "    properly closed by putting this on the line following where it should end.  Intended mostly to aid in development.\r\n");
	strcat (buffer, "(# paramcount [int])\r\n");
	strcat (buffer, "    Specifies that the number of parameters given to the script must be at least equal to the given int.  This is just a shorthand for\r\n");
	strcat (buffer, "    checking to see if every individual $x parameter you need is defined.  If the required number of parameters were not given, the\r\n");
	strcat (buffer, "    script fails.\r\n");
	strcat (buffer, "(# skip [label])\r\n");
	strcat (buffer, "    Skips forward to the given label.  An error is raised if eof is hit before the label is.  Note that this overwrites the buffer\r\n");
	strcat (buffer, "    used by error.skip, so you will need to have (# error.skip) again after (# skip) if you want it to keep working.\r\n");
	strcat (buffer, "(# goto [label])\r\n");
	strcat (buffer, "    Resets the file pointer to the beginning of the script file and then skips forward until [label] is found.\r\n");
	strcat (buffer, "(# gotocount [int])\r\n");
	strcat (buffer, "    Sets the goto count.  If more gotos than this are executed, infinite loop is assumed, and the script is aborted.  The default\r\n");
	strcat (buffer, "    is 256.  Misuse of this command can defeat the infinite loop detector; use with care.\r\n");

	if (buffer[4095] != (char)0xED)
	{
		MessageBox (NULL, "Warning--buffer overflow, holy McCrapBottles, batman!", "BEJEEZUS!", MB_ICONWARNING);
		MessageBox (NULL, buffer, "Script quick reference", 0);
		ExitProcess (0);
	}

	MessageBox (NULL, buffer, "Script quick reference", 0);

}
#endif

//strip off newlines
void ScrStrip (char *a)
{
	while (*a && *a != '\r' && *a != '\n')
		a++;
	*a = '\0';
}

//strip off the ')'.  This is used for (# lines.
void ScrStripParen (char *a)
{
	while (*a && *a != ')')
		a++;
	*a = '\0';
}




//Is this function really any better than having it all stuck in the original?
//this function is used by RopBegin for the (begin) function
DERR ScrHashProcessing (char *buffer, int *incomment, char *labelbuf, char **targetlabel, int *restart,
						int *errormode, int nesting, int *abort, int paramc, char *error, int errlen, int *gotocount)
{
	char *temp;
	int argc;
	char **argv = NULL;
	int st;

	*restart = 0; //always set this to zero, unless we hit a "goto" below

	temp = strstr (buffer, "(#");
	fail_if (!temp, SCRIPT_INSANE_HASH_DETECT, 0); //we check for the existance of (# in the IsHashLine func, so if it's not here, something's gone crazy

	ScrStripParen (temp);

	st = StrTokenize (temp + 2, &argc, &argv);
	fail_if (!st, SCRIPT_TOKENIZE_FAILED, 0);

	if (strcmp (argv[0], "label") == 0)
	{
		fail_if (argc < 2, SCRIPT_INVALID_LABEL, 0);
		if (*targetlabel && strcmp (*targetlabel, argv[1]) == 0)
			*targetlabel = NULL;

		//explanation:  a label only means anything if we are currently looking for one.
		//This happens when the user gives a label to the (script) command, or we are in
		//error.label mode and an error has occured.  The flag for knowing we're in a
		//label skip mode is having *targetlabel == '\0'.
	}
	else if (!*targetlabel)
	{
		if (strcmp (argv[0], "return") == 0)
		{
			//end right now with success status
			fail_if (TRUE, PF_SUCCESS, 0);
		}
		else if (strcmp (argv[0], "abort") == 0)
		{
			//end right now with failure status.
			//copy an error string out if one was given
			if (argc >= 2)
			{	
				strncpy (error, argv[1], errlen - 1);
				error[errlen - 1] = '\0';
			}
			fail_if (TRUE, SCRIPT_ABORTED_BY_HASH, 0);
		}
		else if (strcmp (argv[0], "comment.in") == 0)
		{
			*incomment = 1;
		}
		else if (strcmp (argv[0], "comment.out") == 0)
		{
			*incomment = 0;
		}
		else if (strcmp (argv[0], "base") == 0)
		{
			fail_if (nesting != 0, SCRIPT_BASE_NESTING_ERROR, 0);
		}
		else if (strcmp (argv[0], "error.abort") == 0)
		{
			*errormode = ERROR_ABORT;
		}
		else if (strcmp (argv[0], "error.cont") == 0)
		{
			*errormode = 0;
		}
		else if (strcmp (argv[0], "error.vcont") == 0)
		{
			*errormode = ERROR_VERBOSE;
		}
		else if (strcmp (argv[0], "error.skip") == 0)
		{
			fail_if (argc < 2, SCRIPT_INVALID_SKIP_LABEL, 0);
			strncpy (labelbuf, argv[1], MAX_LABEL - 1);
			labelbuf[MAX_LABEL - 1] = '\0';

			*errormode = ERROR_SKIP;
		}
		else if (strcmp (argv[0], "error.vskip") == 0)
		{
			fail_if (argc < 2, SCRIPT_INVALID_SKIP_LABEL, 0);
			strncpy (labelbuf, argv[1], MAX_LABEL - 1);
			labelbuf[MAX_LABEL - 1] = '\0';

			*errormode = ERROR_SKIP | ERROR_VERBOSE;
		}
		else if (strcmp (argv[0], "paramcount") == 0)
		{
			fail_if (argc < 2, SCRIPT_INVALID_COUNT_LINE, 0);
			fail_if (paramc < atoi (argv[1]), SCRIPT_INSUFFICIENT_PARAMETERS, 0);
		}
		else if (strcmp (argv[0], "skip") == 0)
		{
			fail_if (argc < 2, SCRIPT_INVALID_SKIP_LABEL, 0);
			strncpy (labelbuf, argv[1], MAX_LABEL - 1);
			labelbuf[MAX_LABEL - 1] = '\0';
			*targetlabel = labelbuf;
		}
		else if (strcmp (argv[0], "goto") == 0)
		{
			fail_if (argc < 2, SCRIPT_INVALID_SKIP_LABEL, 0);
			strncpy (labelbuf, argv[1], MAX_LABEL - 1);
			labelbuf[MAX_LABEL - 1] = '\0';
			*targetlabel = labelbuf;

			//goto starts the search from the beginning of the file
			(*gotocount)--;
			fail_if (!(*gotocount), SCRIPT_INFINITE_LOOP, 0);

			*restart = 1;
		}
		else if (strcmp (argv[0], "gotocount") == 0)
		{
			fail_if (argc < 2, SCRIPT_INVALID_GOTOCOUNT_LINE, 0);
			*gotocount = atoi (argv[1]);
		}
		else
		{
			fail_if (TRUE, SCRIPT_INVALID_HASH_LINE, 0);
		}
	} //if (!targetlabel)


	//done with hash line processing.
	StrFreeTokenize (argc, argv);

	*abort = 0;

	//never pass hash lines to the parse functions.
	return PF_SUCCESS;

failure:

	if (argv)
		StrFreeTokenize (argc, argv);

	*abort = 1;
	return st;
}




DERR ScrExecute (char *filename, char *targetlabel, int *lineout, char *error, int errlen, int paramc, char **paramv)
{
	//int argc;
	//char **argv = NULL;
	FILE *f = NULL;
	parser_t *par = NULL;
	DERR st;
	char buffer[1024];
	char labelbuf[MAX_LABEL];
	int nesting;
	int incomment;
	int linenumber = -1;
	int errormode;
	char *output;
	int x;
	int restart;
	int gotocount = 256; //used for infinite loop detection.

	if (lineout)
		*lineout = -1;


	if (!error)
	{
		error = _alloca (1024);
		errlen = 1024;
	}

	st = RopInherit (&par);
	fail_if (!DERR_OK (st), st, 0);

	st = ParAddAtom (par, "$0", filename, 0);
	fail_if (!DERR_OK (st), st, 0);

	//add the parameters, if they exist
	for (x=0;x<paramc;x++)
	{
		_snprintf (labelbuf, MAX_LABEL - 1, "$%i", x + 1);
		labelbuf[MAX_LABEL - 1] = '\0'; //i STRONGLY doubt this will ever be necessary, but whatever
		st = ParAddAtom (par, labelbuf, paramv[x], 0);
		fail_if (!DERR_OK (st), st, 0);
	}
	labelbuf[0] = '\0';


	f = fopen (filename, "r");
	fail_if (!f, SCRIPT_FOPEN_FAILED, DI (errno));

	incomment = 0;
	nesting = 0;
	linenumber = 0;
	errormode = ERROR_ABORT;

	while (fgets (buffer, 1024, f))
	{
		ScrStrip (buffer);

		linenumber++;

		if (strncmp (buffer, "//", 2) == 0)
			continue;

		if (ScrIsHashLine (buffer))
		{
			st = ScrHashProcessing (buffer, &incomment, labelbuf, &targetlabel, &restart, &errormode, nesting, &x, paramc, error, errlen, &gotocount);
			fail_if (!DERR_OK (st) || x, st, 0); //x is the abort var from ScrHashProcessing

			if (restart)
			{
				linenumber = 0;
				st = fseek (f, 0, SEEK_SET);
				fail_if (st, SCRIPT_FSEEK_FAILED, 0);
			}

			continue;
		}

		//if we're in a multiline comment, keep going
		if (incomment)
			continue;

		//if we're in search of a label, keep going
		if (targetlabel)
			continue;

		//ok, so we have something we want to parse.

		error[0] = '\0';
		st = ParRunLine (par, buffer, &output, &nesting, error, errlen);
		if (output)
			ParFree (output); //we definitely don't care what the output was.

		if (!DERR_OK (st))
		{
			ParReset (par);

			//if there was a failure, do what the error mode tells us to do.
			if (errormode & ERROR_VERBOSE)
				ConWriteStringf ("Script line %i error: %s\n", linenumber, error);

			fail_if (errormode & ERROR_ABORT, SCRIPT_ABORTED_DUE_TO_ERROR, 0);

			//if ERROR_SKIP, set the skip target
			if (errormode & ERROR_SKIP)
				targetlabel = labelbuf;

			st = ParAddAtom (par, "lasterror", error, 0);
			fail_if (!DERR_OK (st), st, 0);

			//if they're continuing, null out the error string.  Otherwise,
			//if it gets to the end of the file and would have said
			//LABEL_NOT_FOUND, it will print out the error from whatever
			//caused the label skip in the first place.
			*error = '\0';
		}

	} //while (fgets)

	fail_if (!feof (f), SCRIPT_FILE_NOT_EOF, 0);
	fail_if (ferror (f), SCRIPT_FILE_ERROR, DI (ferror (f)));
	fail_if (targetlabel, SCRIPT_LABEL_NOT_FOUND, 0);
	fail_if (nesting, SCRIPT_UNTERMINATED_COMMAND, 0);

	//fail_if (argv, SCRIPT_WTF, 0);

	fclose (f);
	ParDestroy (par);

	return PF_SUCCESS;

failure:

	//if (argv)
	//	StrFreeTokenize (argc, argv);

	if (f)
		fclose (f);

	if (par)
		ParDestroy (par);

	if (lineout)
		*lineout = linenumber;

	return st;

}








//////////////////////// Script commands ///////////////////////////

int ScrVerifyParamv (int paramc, char **paramv)
{
	int x;

	__try
	{
		for (x=0;x<paramc;x++)
		{
			if ((((DWORD)paramv[x]) & 0xFFFF0000) == 0)
				return 0; //if there's nothing in the high bits, definitely not a string.
			if (strlen (paramv[x]) > 1024) //exceptions happen here
				return 0;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}

	return 1;
}


void ScrScript (parerror_t *pe, char *script, int count, ...)
{
	char **paramv;
	int x;
	int res, st;
	va_list va;
	char error[1024];
	int linenum = -1;

	fail_if (!script, 0, PE_BAD_PARAM (1));

	paramv = _alloca (sizeof (char *) * (count - 3));
	va_start (va, count);
	for (x=0;x<count - 3;x++)
	{
		paramv[x] = va_arg (va, char *);
	}
	va_end (va);

	fail_if (!ScrVerifyParamv (count - 3, paramv), 0, PE ("Invalid (non-string) parameter passed to (script)", 0, 0, 0, 0));

	error[0] = '\0';
	res = ScrExecute (script, NULL, &linenum, error, 1024, count - 3, paramv);
	fail_if (!DERR_OK (res), res, PE (NULL, 0, 0, 0, 0));

	return;

failure:

	if (!pe->format)
	{
		pe->format = "line:%i: %s";
		pe->param1 = linenum;
		if (error[0])
		{
			pe->param2 = (int)ParMalloc (strlen (error) + 1);
			if (!pe->param2)
			{
				pe->param2 = (int)"(Out of memory allocating error print string)";
			}
			else
			{
				strcpy ((char *)pe->param2, error);
				pe->flags |= PEF_PARAM2;
			}
		}
		else
		{
			pe->param2 = (int)PfDerrString (st);
		}
	}

	return;
}


void ScrScriptSub (parerror_t *pe, char *script, char *subscript, int count, ...)
{
	char **paramv;
	int x;
	int res, st;
	va_list va;
	char error[1024];
	int linenum = -1;

	fail_if (!script, 0, PE_BAD_PARAM (1));
	fail_if (!subscript, 0, PE_BAD_PARAM (2));

	paramv = _alloca (sizeof (char *) * (count - 4));
	va_start (va, count);
	for (x=0;x<count - 4;x++)
	{
		paramv[x] = va_arg (va, char *);
	}
	va_end (va);

	fail_if (!ScrVerifyParamv (count - 4, paramv), 0, PE ("Invalid (non-string) parameter passed to (script)", 0, 0, 0, 0));

	error[0] = '\0';
	res = ScrExecute (script, subscript, &linenum, error, 1024, count - 4, paramv);
	fail_if (!DERR_OK (res), res, PE (NULL, 0, 0, 0, 0));

	return;

failure:

	if (!pe->format)
	{
		pe->format = "line:%i: %s";
		pe->param1 = linenum;
		if (error[0])
		{
			pe->param2 = (int)ParMalloc (strlen (error) + 1);
			if (!pe->param2)
			{
				pe->param2 = (int)"(Out of memory allocating error print string)";
			}
			else
			{
				strcpy ((char *)pe->param2, error);
				pe->flags |= PEF_PARAM2;
			}
		}
		else
		{
			pe->param2 = (int)PfDerrString (st);
		}
	}

	return;
}

DERR ScrAddGlobalCommands (parser_t *root)
{
	int st;

	st = ParAddFunction (root, "p5.scr.run", "vesc.", ScrScript, "Executes a p5 script", "(script [s:filename] [s:param1] [s:param2] ...) = [v]", "Parameters are optional.  The script can access them through $1, $2, etc.  See (script.help) for more information on script formatting.  All parameters MUST be strings or atoms.  The maximum length of a parameter is clamped at 1024 bytes as an effort to avoid invalid string pointers getting passed to the script.  See (info \"script\") for more information.");
	fail_if (!DERR_OK (st), st, 0);

	st = ParAddFunction (root, "p5.scr.runsub", "vessc.", ScrScriptSub, "Executes a p5 script, beginning at the specified label", "(script.sub [s:filename] [s:label] [s:param1] [s:param2] ...) = [v]", "Parameters are optional.  The script can access them through $1, $2, etc.  See (script.help) for more information on script formatting.  All parameters MUST be strings or atoms.  The maximum length of a parameter is clamped at 1024 bytes as an effort to avoid invalid string pointers getting passed to the script.  See (info \"script\") for more information.");
	fail_if (!DERR_OK (st), st, 0);

	//st = ParAddFunction (root, "p5.scr.help", "v", ScrHelpWindow, "Shows the script syntax help window", "(script.help) = [v]", "Contains information on the (#) special script control command.");
	//fail_if (!DERR_OK (st), st, 0);

	st = ParAddAlias (root, "script",  "vesc.", "p5.scr.run");	fail_if (!DERR_OK (st), st, 0);

	return PF_SUCCESS;
failure:
	return st;
}

