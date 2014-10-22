#include <stdio.h>
#include <stdlib.h>
#include "cgitool.h"

main()
{
   int i,j,len;
   char *inbuf;
   formfield *fields;
   int count;
   char *contlen, *conttype;
   unsigned char *dat;

   printf("Content-type: text/html\n");
   printf("\n");
   contlen=getenv("CONTENT_LENGTH");
   if(contlen==NULL)
      return;

   printf("<html>hello<br>\n");

   conttype=getenv("CONTENT_TYPE");

   printf("%s<br>\n",conttype);

   /*
   readCGIdata(&inbuf,&len);
   */
   
   extractCGIinput(&fields, 20, &count);

   printf("%d<br>\n",count);

   for(i=0; i<count; i++) {
     printf("field: %s<br>\n",fields[i].name);
     printf("attr: %s<br>\n",fields[i].attr);

     dat=fields[i].data;
     for(j=0; j<fields[i].len; j++)
       if(dat[j]=='\n')
	 printf("\\n<br>\n");
       else if(dat[j]=='\r')
	 printf("\\r");
       else if(dat[j]<32)
	 printf(".");
       else if(dat[j]=='<')
	 printf("&lt;");
       else if(dat[j]=='>')
	 printf("&gt;");
       else
	 printf("%c",dat[j]);
     printf("<br>\n---------------------------<br>\n");
   }


   /*
   extractCGIinput(inbuf,&fields,&data,&count);

   for(i=0; i<count; i++) 
      printf("%s-%s<br>\n",fields[i],data[i]);

   for(i=0; i<count; i++) {
      free(fields[i]);
      free(data[i]);
   }
   free(fields);
   free(data);
   free(inbuf);
   */
   printf("<br></html>\n");
}

