#include "tmpl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strlist.h"

#define MARKER     "<!--template:"
#define MARKER_END  "--!>"
#define MARKER_LEN  13

static char *getdata(char *index, char **field, char **data, int fieldcount)
{
  int i;

  i=0;
  while(i<fieldcount) {
    if(strcmp(field[i],index)==0) 
      return data[i];
    i++;
  }  
  return "";
}

static char *extractfieldname(char *line, char *fldname)
{
  char *ending;
  int flen;

  *fldname=0;
  line+=MARKER_LEN;
  if((ending=strstr(line,MARKER_END))==NULL)
    return fldname;
  flen=ending-line;
  strncpy(fldname,line,flen);
  fldname[flen]=0;
  return fldname;
}

char *putintemplate(char *tmplname, int fieldcount, 
		    char **fields, char **data)
{
  char *outstring;

  FILE *tin;

  if((tin=fopen(tmplname,"r"))==NULL) {
    return strdup("");
  }

  outstring=putinftemplate(tin, fieldcount, fields, data);

  fclose(tin);

  return outstring;
}

char *putinftemplate(FILE *tin, int fieldcount,
		     char **fields, char **data)
{
  struct strlist outlist;
  char line[200];
  char fldname[100];
  char *outstring;

  newstrlist(&outlist);

  while(fgets(line,200,tin)!=NULL) {
    if(strncmp(line,MARKER,MARKER_LEN)==0) {
      addtostrlist(&outlist,getdata(extractfieldname(line, fldname), 
				    fields, data, fieldcount));
/*      addtostrlist(&outlist,"\n"); */
    } else {
      addtostrlist(&outlist,strdup(line));
    }
  }

  outstring=strlisttostring(&outlist);
  deletestrlist(&outlist);

  return outstring;
}

