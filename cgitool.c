#include "cgitool.h"
#include <stdlib.h>
#include <string.h>

static char *decodeCGIstr(char *str)
{
  char *in;
  char *out;
  char c1,c2;

  in=out=str;
  while(*in!=0) {
    if(*in=='+')
      *out=' ';
    else if(*in=='%') {
      c1=*(in+1);
      c1-='0';
      if(c1>15)
	c1-='A'-'0'-10;
      if(c1>15)
	c1-='a'-'A'-'0'-10;
      c2=*(in+2);
      c2-='0';
      if(c2>15)
	c2-='A'-'0'-10;
      if(c2>15)
	c2-='a'-'A'-'0'-10;
      *out=c1*16+c2;
      in+=2;
    } else
      *out=*in;
    in++;
    out++;
  }
  *out=0;

  return str;
}

void readCGIdata(unsigned char **buffer, int *len)
{
  char *contlen;
  int inlen,i;

  contlen=getenv("CONTENT_LENGTH");

  if(contlen==NULL) {
    *buffer=(char *)malloc(0);
    return;
  }
  
  inlen=atoi(contlen);
  *buffer=(char *)malloc(inlen+1);

  if(*buffer==NULL) {
    *buffer=(char *)malloc(0);
    return;
  }

  i=0;
  for(i=0; i<inlen; i++) 
    (*buffer)[i]=getchar();
  (*buffer)[i]=0;

  if(len!=NULL)
    *len=inlen;
}

static void extrURLencdata(unsigned char *posteddata, formfield **fields, 
			  int maxcount, int *count)
{
  formfield *temparray;
  formfield *tempfield;

  char *st=strdup(posteddata);
  char *p, *s, *t;
  int i;
  
  temparray=(formfield *)malloc(maxcount*sizeof(formfield));

  *count=0;
  p=st;
  while((*p!=0) && (*count<maxcount)) {
    (*count)++;
    tempfield=&temparray[*count-1];
    s=p;
    t=s;
    while((*t!='=') && (*t!=0))
      t++;
    if(*t!=0) {
      *t=0;
      tempfield->name=strdup(s);
    } else {
      tempfield->name=strdup(s);
      tempfield->attr=strdup("");
      tempfield->len=0;
      tempfield->data=strdup("");
      break;
    }
    p=t+1;
    s=p;
    t=s;
    while((*t!='&') && (*t!=0))
      t++;
    if(*t!=0) {
      *t=0;
      decodeCGIstr(s);
      tempfield->attr=strdup("");
      tempfield->len=strlen(s);
      tempfield->data=strdup(s);
    } else {
      decodeCGIstr(s);
      tempfield->attr=strdup("");
      tempfield->len=strlen(s);
      tempfield->data=strdup(s);
      break;
    }
    p=t+1;
  }
  free(st);
  
  (*fields)=(formfield *)malloc((*count)*sizeof(formfield));
  for(i=0; i<*count; i++)
    (*fields)[i]=temparray[i];
  free(temparray);
}

static char *getpartsep(char *conttype)
{
  char *bmark="boundary=";
  char *sstart;

  sstart=strstr(conttype,bmark);
  return strdup(sstart+strlen(bmark));
}

static int ispartsep(unsigned char *st, char *partsep)
{
  if((st[0]!='-') || (st[1]!='-'))
    return 0;

  st+=2;
  while(*partsep!=0) {
    if(*partsep!=*st)
      return 0;
    partsep++;
    st++;
  }
  return 1;
}

static char *getfieldinfo(char *st, char **name, char **attr)
{
  char *marker="Content-Disposition: form-data; name=\"";
  char *pstart, *pend;
  int len;
  char *fname;

  if(strstr(st,marker)!=st) {
    (*name)=strdup("");
    (*attr)=strdup("");
    return *name;
  }
  pstart=st+strlen(marker);
  pend=pstart;
  while((*pend!=0) && (*pend!='\"'))
    pend++;
  len=pend-pstart;
  (*name)=(char *)malloc(len+1);
  strncpy((*name),pstart,len);
  (*name)[len]=0;

  /* extract attribute */
  while((*pend!=';') && (*pend!='\r'))
    pend++;
  if(*pend=='\r')
    (*attr)=strdup("");
  else {
    pstart=pend+1;
    while(*pstart==' ')
      pstart++;
    pend=pstart;
    while(*pend!='\r')
      pend++;
    len=pend-pstart;
    (*attr)=(char *)malloc(len+1);
    strncpy((*attr),pstart,len);
    (*attr)[len]=0;
  }
  return *name;
}

static unsigned char *skipsep(unsigned char *buff, char *partsep)
{
  if(ispartsep(buff,partsep)) {
    buff+=strlen(partsep)+2;
    if((buff[0]=='-') && (buff[1]=='-'))
      return NULL;
    else
      return buff+2;  /* add another 2 for \r\n  */
  } else
    return NULL;     
}

static unsigned char *skippartheader(unsigned char *buff)
{
  char *lend;

  while((buff[0]!='\r') || (buff[1]!='\n')) {
    lend=buff;
    while((lend[0]!=0) && ((lend[0]!='\r') || (lend[1]!='\n')))
      lend++;
    buff=lend+2;
  }
  return buff+2;
}

static unsigned char *skipdata(unsigned char *dstart, char *partsep)
{
  unsigned char *lend;

  lend=dstart;
  do {
    while((lend[0]!='\r') || (lend[1]!='\n'))
      lend++;
    lend+=2;
  } while(!ispartsep(lend,partsep));

  return lend-2;
}

static void extrmultipartdata(unsigned char *inbuf, char *partsep,
			      formfield **fields, int maxcount,int *count)
{
  formfield *temparray;
  formfield *tempfield;
  unsigned char *p;
  unsigned char *dstart, *dend;
  int i,len;

  temparray=(formfield *)malloc(maxcount*sizeof(formfield));
  p=inbuf;
  *count=0;
  while(((p=skipsep(p, partsep))!=NULL) && (*count<maxcount)) {
    (*count)++;
    tempfield=&temparray[*count-1];

    getfieldinfo(p,&tempfield->name, &tempfield->attr);
    //    printf("f: %s<br>\n",tempfield->name);

    dstart=skippartheader(p);
    //    printf("d: %c%c%c%c<br>\n",dstart[0],dstart[1],dstart[2],dstart[3]);

    dend=skipdata(dstart,partsep);
    len=dend-dstart;

    tempfield->len=len;
    tempfield->data=(unsigned char *)malloc(len+1);
    for(i=0; i<len; i++) 
      tempfield->data[i]=dstart[i];
    tempfield->data[len]=0;

    p=dend+2;
  }
  
  (*fields)=(formfield *)malloc((*count)*sizeof(formfield));
  for(i=0; i<*count; i++)
    (*fields)[i]=temparray[i];
  free(temparray);
}

void extractCGIinput(formfield **fields, int maxcount, int *count)
{
  char *conttype=getenv("CONTENT_TYPE");
  unsigned char *inbuf;
  char *partsep;
  int len;

  readCGIdata(&inbuf,&len);
  if(strcmp(conttype,"application/x-www-form-urlencoded")==0)
    extrURLencdata(inbuf,fields,maxcount,count);
  else {
    *count=0;
    partsep=getpartsep(conttype);
    //    printf("sep: %s<br>\n",partsep);
    extrmultipartdata(inbuf,partsep,fields,maxcount,count);
    free(partsep);
  }
  free(inbuf);
}

