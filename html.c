#include "html.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cgitool.h"

static char *htmltab[]={
  "\n", "<br>\n",
  "\"", "&quot;",
  "&", "&amp;",
  "<", "&lt;",
  ">", "&gt;",
/*  " ", "&nbsp;", */
  0};

static int charmap[256];

void initHTMLencodingtable()
{
  int i;

  for(i=0; i<256; i++)
    charmap[i]=0;

  i=0;
  while(htmltab[i]!=0) {
    charmap[htmltab[i][0]]=i+1;
    i+=2;
  }
}

char *HTMLencode(char *st)
{
  unsigned char *p;
  unsigned char *outstr, *op;
  int len=0;

  p=(unsigned char *)st;
  while(*p!=0) {
    if((*p<0) || (*p>255))
      len++;
    else if((*p=='\\') && (*(p+1)!=0)) {
      len++;
      p++;
    } else if(charmap[*p]==0)
      len++;
    else
      len+=strlen(htmltab[charmap[*p]]);
    p++;
  }

  outstr=(char *)malloc(len+1);
  p=st;
  op=outstr;
  while(*p!=0) {
    if((*p<0) || (*p>255)) {
      *op=*p;
      op++;
    } else if((*p=='\\') && (*(p+1)!=0)) {
      *op=*(p+1);
      op++;
      p++;
    } else if(charmap[*p]==0) {
      *op=*p;
      op++;
    } else {
      strcpy(op,htmltab[charmap[*p]]);
      op+=strlen(htmltab[charmap[*p]]);
    }
    p++;
  }  

  *op=0;
  return outstr;
}

char *HTMLencode_exclude_br(char *st)
{
  unsigned char *outstr;
  int oldnlmap;

  oldnlmap=charmap['\n'];
  charmap['\n']=0;

  outstr=HTMLencode(st);

  charmap['\n']=oldnlmap;

  return outstr;
}

/*
main()
{
  char line[100];
  char *buf;

  initHTMLencodingtable();
  while(fgets(line,100,stdin)!=NULL){
    decodeCGIstr(line);
    printf("decoded: %s\n",line);  
    buf=HTMLencode(line);
    printf("%s",buf);
    free(buf);
  }
}
*/
