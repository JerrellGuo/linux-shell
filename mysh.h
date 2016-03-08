#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#if defined(__linux__)
    extern	char	*strdup(const char *str);
    extern	int	fileno(const FILE *fp);
#endif

//  Written by Chris.McDonald@uwa.edu.au, October 2015

// ----------------------------------------------------------------------

#define	DEFAULT_HOME	"/tmp"
#define	DEFAULT_PATH	"/bin:/usr/bin:/usr/local/bin:."
#define	DEFAULT_CDPATH	".:.."

#define COMMENT_CHAR	'#'	// comment character
#define HOME_CHAR	'~'	// home directory character

//  ----------------------------------------------------------------------

//  AN enum IN C99 'GENERATES' A SEQUENCE OF UNIQUE, ASCENDING CONSTANTS
typedef enum {
	N_AND = 0,		// as in   cmd1 && cmd2
	N_BACKGROUND,		// as in   cmd1 &
	N_OR,			// as in   cmd1 || cmd2	
	N_SEMICOLON,		// as in   cmd1 ;  cmd2	
	N_PIPE,			// as in   cmd1 |  cmd2	
	N_SUBSHELL,		// as in   ( cmds )
	N_COMMAND		// an actual command node itself
} NODETYPE;


typedef	struct ct {
    NODETYPE	type;		// the type of the node, &&, ||, etc

    int		argc;		// the number of args iff type == N_COMMAND
    char	**argv;		// the NULL terminated argument vector

    char	*infile;	// as in    cmd <  infile
    char	*outfile;	// as in    cmd >  outfile
    bool	append;		// true iff cmd >> outfile

    struct ct	*left, *right;	// pointers to left and right subtrees
} CMDTREE;


extern CMDTREE	*parse_cmdtree(FILE *);		// in parser.c
extern void	free_cmdtree(CMDTREE *);	// in parser.c
extern int	execute_cmdtree(CMDTREE *);	// in execute.c


/* The global variable HOME points to a directory name stored as a
   character string. This directory name is used to indicate two things:

    - the directory used when the  cd  command is requested without arguments.
    - replacing the leading '~' character in args,  e.g.  ~/filename

   The HOME variable is initialized with the value inherited from the
   invoking environment (or DEFAULT_HOME if undefined).
 */

extern	char	*HOME;

/* The global variables PATH and CDPATH point to character strings representing
   colon separated lists of directory names.

   The value of PATH is used to search for executable files when a command
   name does not contain a '/'

   Similarly, CDPATH provides a colon separated list of directory names
   that are used in an attempt to chage the current working directory.
 */

extern	char	*PATH;
extern	char	*CDPATH;

extern	char	*argv0;		// The name of the shell, typically mysh
extern	bool	interactive;	// true if mysh is connected to a 'terminal'


//  ----------------------------------------------------------------------

//  TWO PRE-PROCESSOR MACROS THAT MAY HELP WITH DEBUGGING YOUR CODE.
//      check_allocation(p) ENSURES THAT A POINTER IS NOT NULL, AND
//      print_cmdtree(t)  PRINTS THE REQUESTED COMMAND-TREE
//  THE TWO ACTUAL FUNCTIONS ARE DEFINED IN globals.c

#define	check_allocation(p)	\
	check_allocation0(p, __FILE__, __func__, __LINE__)
extern	void check_allocation0(void *p, char *file, const char *func, int line);

#define	print_cmdtree(t)	\
	printf("called from %s, %s() line %i:\n", __FILE__,__func__,__LINE__); \
	print_cmdtree0(t)
extern	void print_cmdtree0(CMDTREE *t);

