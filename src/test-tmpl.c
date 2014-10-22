#include <stdio.h>
#include <stdarg.h>
#include "tmpl.h"

char **createstrarray(char*** ar, int count, ...)
{
  va_list ap;
  int i;

  va_start(ap, count);
  *ar=(char **)malloc(count*sizeof(char *));
  for(i=0; i<count; i++)
    (*ar)[i]=(char *)va_arg(ap,char *);
  va_end(ap);
  return *ar;
}

main()
{
  char **flist, **vlist;
  char *temp1;

  temp1=putintemplate("testtemp",2,
                      createstrarray(&flist,2,"name","test1"),
                      createstrarray(&vlist,2,"Jittat","Oh My God"));
  free(flist);
  free(vlist);
  printf("%s",putintemplate("testtemp2",2,
                            createstrarray(&flist,1,"test2"),
                            createstrarray(&vlist,1,temp1)));
}

