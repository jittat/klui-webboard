#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <stdarg.h>
#include <string.h>
#include "cgitool.h"
#include "strlist.h"
#include "tmpl.h"

#define BOARDDIR           ""
#define THREADCOUNTFILE    "threadcount"
#define ANNOUNCEMENTFILE   "announcement"
#define MAINTEMPLATE       "main.template"

#define MAXANNOUNTMENT     20

#define THREADPERPAGE      40

static char tempfilename[100];

char *buildfilename(char *fname, char *ext)
{
  strcpy(tempfilename, BOARDDIR);
  strcat(tempfilename, fname);
  strcat(tempfilename, ext);

  return tempfilename;
}

char *trimnl(char *buff)
{
  if(*buff==0)
    return buff;

  buff[strlen(buff)-1]=0;
}

char *buildthreadhead(char *buffer, int id)
{
  int fd;
  FILE *fp;
  char idstr[10];

  char topic[100];
  char name[100];
  char date[10];
  char time[10];
  char cstr[10];
  char pname[100];
  char pdate[10];
  char ptime[10];
  int counter;

  sprintf(idstr,"%d",id);

  fd=open(buildfilename(idstr,".head"),O_RDONLY);
  if(fd==-1) {
    *buffer=0;
    return NULL;
  }

  flock(fd,LOCK_SH);
  fp=fdopen(fd,"r");
  fgets(topic,100,fp);
  fgets(name,100,fp);
  fgets(date,100,fp);
  fgets(time,100,fp);

  trimnl(topic);
  trimnl(name);
  trimnl(date);
  trimnl(time);

  fgets(cstr,10,fp);
  sscanf(cstr,"%d",&counter);

  if(counter==0) {
    sprintf(buffer,"%s: <a href=\"read.cgi?%s.html?0\" target=\"_blank\">%s</a> - %s [%s] (0)",
	    idstr, idstr, topic, name, date);
  } else {
    fgets(pname,100,fp);
    fgets(pdate,100,fp);
    fgets(ptime,100,fp);

    trimnl(pname);
    trimnl(pdate);
    trimnl(ptime);

    if(counter>=10)
      sprintf(buffer,"%s: <a href=\"read.cgi?%s.html?%d\" target=\"_blank\">%s</a> <img src=\"../webpicture/vhot.gif\"> - %s [%s] (%d - %s %s %s)",
  	      idstr, idstr, counter, topic, name, date,
  	      counter,pname,ptime,pdate);
    else
      sprintf(buffer,"%s: <a href=\"read.cgi?%s.html?%d\" target=\"_blank\">%s</a> - %s [%s] (%d - %s %s %s)",
  	      idstr, idstr, counter, topic, name, date,
  	      counter,pname,ptime,pdate);
  }

  fclose(fp);
  flock(fd,LOCK_UN);
  close(fd);

  return buffer;
}

int readthreadcount()
{
  int fd;
  FILE *fp;
  int value;

  fd=open(buildfilename(THREADCOUNTFILE,""),O_RDONLY);
  if(fd==-1)
    return 0;

  flock(fd,LOCK_SH);
  fp=fdopen(fd,"r");
  fscanf(fp,"%d",&value);

  fclose(fp);
  flock(fd,LOCK_UN);
  close(fd);

  return value;
}

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

char *generatepagelist(int pcount, int cpage)
{
  struct strlist plinklist;
  int i;
  char pstr[100];

  newstrlist(&plinklist);
  for(i=0; i<pcount; i++) {
    if(cpage!=i+1) {
      sprintf(pstr,"<a href=\"webboard.cgi?%d\">%d</a> ",i+1,i+1);
      addtostrlist(&plinklist,strdup(pstr));
    } else {
      sprintf(pstr,"%d ",i+1);
      addtostrlist(&plinklist,strdup(pstr));
    }
  }
  return strlisttostring(&plinklist);
}

void readannouncement(int *count, int *list)
{
  FILE *fp;
  int i;

  fp=fopen(buildfilename(ANNOUNCEMENTFILE,""),"r");
  if(fp==NULL) {
    *count=0;
    return;
  }
  fscanf(fp,"%d",count);
  for(i=0; i<*count; i++)
    fscanf(fp,"%d",&list[i]);

  fclose(fp);
}

main(int argc, char *argv[])
{
  int tcount;
  int annlist[MAXANNOUNTMENT];
  int anncount;
  int i,j;
  struct strlist threadlist;
  struct strlist annthreadlist;
  char thead[300];
  char *allthead;
  char *annthead;
  char *pagelink;
  char **flist;
  char **vlist;
  int page,pagecount;
  int tstart,tend;

  if(argc!=2) {
    page=1;
  } else {
    sscanf(argv[1],"%d",&page);
    if(page==0)
      page=1;
  }

  newstrlist(&threadlist);
  newstrlist(&annthreadlist);

  tcount=readthreadcount();
  readannouncement(&anncount,annlist);

  tstart=tcount-(page-1)*THREADPERPAGE;
  tend=tstart-THREADPERPAGE+1;

  if(tstart<1) tstart=tcount;
  if(tend<1) tend=1;

  for(i=0; i<anncount; i++) 
    if(buildthreadhead(thead,annlist[i])!=NULL) {
      strcat(thead,"<br>\n");
      addtostrlist(&annthreadlist, strdup(thead));
    }

  for(i=tstart; i>=tend; i--) {
    if(buildthreadhead(thead,i)!=NULL) {
      strcat(thead,"<br>\n");

      for(j=0; j<anncount & annlist[j]!=i; j++)
	;
      if((j>anncount) || (annlist[j]!=i))
	addtostrlist(&threadlist, strdup(thead));
    }
  }
  allthead=strlisttostring(&threadlist);
  annthead=strlisttostring(&annthreadlist);

  pagecount=(tcount+THREADPERPAGE-1)/THREADPERPAGE; 
  pagelink=generatepagelist(pagecount,page);

  printf("Content-type: text/html\n");
  printf("\n");

  printf("%s",putintemplate(buildfilename(MAINTEMPLATE,""),3,
			    createstrarray(&flist,3,"ann-thread-list",
					   "thread-list","page-links"),
			    createstrarray(&vlist,3,annthead,
					   allthead,pagelink)));

  /* so many thing left to clear up... ignored... sorry */
}

