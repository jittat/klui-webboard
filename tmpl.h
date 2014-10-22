#ifndef TMPL_H_INCLUDED
#define TMPL_H_INCLUDED

#include <stdio.h>

char *putintemplate(char *tmplname, int fieldcount, 
		    char **field, char **data);

char *putinftemplate(FILE *fp, int fieldcount, 
		     char **field, char **data);

#endif

