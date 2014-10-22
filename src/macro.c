#include "macro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strlist.h"

#define MAXCMDLEN  10

static char *basecmdlist[] = {"link", "image", "emph", "verbatim", 
                              "center", NULL};

int findcommand(char *cmdlist[], char *cmd)
{
  int i=0;

  while(cmdlist[i]!=NULL) {
    if(strcmp(cmd,cmdlist[i])==0)
      return i;
    i++;
  }
  return -1;
}

int extractcommand(char *cmdlist[], char *st, char **argstart, char **argend)
{
  char *p,*cp;
  char cmdstr[MAXCMDLEN+1];
  int cmd, i, pcount;

  i=0;
  p=st;
  cp=cmdstr;
  while((*p!='{') && (*p!=0) && (!isspace(*p)) && (i<MAXCMDLEN)) {
    *cp=*p;
    cp++;
    p++;
    i++;
  }
  *cp=0;
  if((cmd=findcommand(cmdlist,cmdstr))!=-1) {
    if(*p=='{') {
      pcount=1;
      p++;
      *argstart=p;
      while(((*p!='}') || (pcount!=1)) && (*p!='\n') && (*p!=0)) { 
        if(*p=='{')
          pcount++;
        else if(*p=='}')
          pcount--;
        p++;
      }
      *argend=p;
    } else {
      *argstart=p;
      *argend=p;
    }
  }
  return cmd;
}

char *appendstr(char *dest, char *src)
{
  while(*src!=0) {
    *dest=*src;
    src++;
    dest++;
  }
  return dest;
}

char *expandmacro(char *st)
{
  char *buffer;
  char *pin, *pout;
  int cmd;
  char *argstart, *argend;
  char temp;
  char *remsg;

  //the size of buffer will never be larger than 100+4*len(st), I hope.
  buffer=malloc(100+strlen(st)*4);

  pin=st;
  pout=buffer;

  while(*pin!=0) {
    if(*pin!='\\') {
      *pout=*pin;
      pin++;
      pout++;
    } else {
      cmd=extractcommand(basecmdlist,pin+1,&argstart,&argend);
      if(cmd==-1) {
        *pout='\\';
        *(pout+1)='\\';
        pout+=2;
        pin++;
      } else {
        temp=*argend;
        *argend=0;

        switch(cmd) {
          case 0:
            pout=appendstr(pout,"\\<a href=\\\"");
            pout=appendstr(pout,argstart);
            pout=appendstr(pout,"\\\"\\>");
            pout=appendstr(pout,argstart);
            pout=appendstr(pout,"\\</a\\>");
            break;
          case 1:
            pout=appendstr(pout,"\\<img src=\\\"");
            pout=appendstr(pout,argstart);
            pout=appendstr(pout,"\\\"\\>");
            break;
          case 2:
            remsg=expandmacro(argstart);
            pout=appendstr(pout,"\\<b\\>");
            pout=appendstr(pout,remsg);
            pout=appendstr(pout,"\\</b\\>");
            free(remsg);
            break;
          case 3:
            pout=appendstr(pout,argstart);
            break;
          case 4:
            remsg=expandmacro(argstart);
            pout=appendstr(pout,"\\<center\\>");
            pout=appendstr(pout,remsg);
            pout=appendstr(pout,"\\</center\\>");
            free(remsg);
            break;
        }

        *argend=temp;
        if(*argend=='}')
          pin=argend+1;
        else
          pin=argend;
      }
    }
  } 
  *pout=0;

  return buffer;
}


char *expandbackslash(char *st)
{
  char *buffer;
  char *pin, *pout;

  //the size of buffer will never be larger than 2*len(st), I hope.
  buffer=malloc(strlen(st)*2+1);

  pin=st;
  pout=buffer;

  while(*pin!=0) {
    if(*pin!='\\') {
      *pout=*pin;
      pin++;
      pout++;
    } else {
      *pout='\\';
      *(pout+1)='\\';
      pout+=2;
      pin++;
    }
  } 
  *pout=0;

  return buffer;
}

/*
main()
{
  char *buff;
  char line[200];

  while(fgets(line,200,stdin)!=NULL){
    buff=expandmacro(line);
    printf("%s",buff);
    free(buff);
  }
}
*/

