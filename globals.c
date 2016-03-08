#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name(s):             student-name1 (, student-name2)
   Student number(s):   student-number-1 (, student-number-2)
   Date:                date-of-submission
 */


//  THREE INTERNAL VARIABLES (SEE mysh.h FOR EXPLANATION)
char	*HOME, *PATH, *CDPATH;

char	*argv0		= NULL;		// the program's name
bool	interactive	= false;

// ------------------------------------------------------------------------

void check_allocation0(void *p, char *file, const char *func, int line)
{
    if(p == NULL) {
	fprintf(stderr, "%s, %s() line %i: unable to allocate memory\n",
			file, func, line);
	exit(2);
    }
}

static void print_redirection(CMDTREE *t)
{
    if(t->infile != NULL)
	printf("< %s ", t->infile);
    if(t->outfile != NULL) {
	if(t->append == false)
	    printf(">");
	else
	    printf(">>");
	printf(" %s ", t->outfile);
    }
}

void print_cmdtree0(CMDTREE *t)
{
    if(t == NULL) {
	printf("<nullcmd> ");
	return;
    }

    switch (t->type) {
    case N_COMMAND :
	printf("command\n");
	for(int a=0 ; a<t->argc ; a++)
	    printf("%s ", t->argv[a]);
	print_redirection(t);
	break;

    case N_SUBSHELL :
	printf("and\n");
	printf("( "); print_cmdtree0(t->left); printf(") ");
	print_redirection(t);
	break;

    case N_AND :
	printf("and\n");
	print_cmdtree0(t->left); printf("&& "); print_cmdtree0(t->right);
	break;

    case N_OR :
	printf("or\n");
	print_cmdtree0(t->left); printf("|| "); print_cmdtree0(t->right);
	break;

    case N_PIPE :
	printf("pipe\n");
	print_cmdtree0(t->left); printf("| "); print_cmdtree0(t->right);
	break;

    case N_SEMICOLON :
	printf("semicolon\n");
	print_cmdtree0(t->left); printf("; "); print_cmdtree0(t->right);
	break;

    case N_BACKGROUND :
	printf("background\n");
	print_cmdtree0(t->left); printf("& "); print_cmdtree0(t->right);
	break;

    default :
	fprintf(stderr, "%s: invalid NODETYPE in print_cmdtree0()\n", argv0);
	exit(1);
	break;
    }
}
