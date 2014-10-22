#include <stdio.h>
#include <stdlib.h>
#include "cgitool.h"

main()
{
   int i,len;
   char *dat;
   int count;
   char *contlen, *conttype;

   printf("Content-type: text/html\n");
   printf("\n");
   contlen=getenv("CONTENT_LENGTH");
   if(contlen==NULL)
      return;

   printf("<html>hello<br>\n");

   conttype=getenv("CONTENT_TYPE");

   printf("content type:%s<br>\n",conttype);

   readCGIdata(&dat,&len);
   
   printf("length: %d<br>\n",len);

   for(i=0; i<len; i++)
     if(dat[i]=='\n')
       printf("\\n<br>\n");
     else if(dat[i]=='\r')
       printf("\\r");
     else if(dat[i]<32)
       printf(".");
     else if(dat[i]=='<')
       printf("&lt;");
     else if(dat[i]=='>')
       printf("&gt;");
     else
       printf("%c",dat[i]);

   printf("<br></html>\n");

   free(dat);
}

