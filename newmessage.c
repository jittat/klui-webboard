#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "cgitool.h"
#include "strlist.h"
#include "tmpl.h"
#include "html.h"

#define BOARDDIR               ""
#define THREADCOUNTFILE        "threadcount"
#define BODYTEMPLATE           "newmessagebody.template"
#define NEWMESSAGETEMPLATE     "newmessage.template"
#define NEWMESSAGEPLACEHOLDER  "<!--template:new-message--!>"
#define MESSAGEINFOFORMAT      "<input name=\"msgid\" type=\"hidden\" value=\"%d\">"
#define REFRESHTEMPLATE        "refresh.template"

static char tempfilename[100];

int threadcount;
FILE *tcountfp;
int tcountfd;

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

void getthreadcount()
{
  tcountfd=open(buildfilename(THREADCOUNTFILE,""),O_RDWR);
  if(tcountfd==-1) {
    tcountfd=open(buildfilename(THREADCOUNTFILE,""),O_RDWR|O_CREAT);
    flock(tcountfd,LOCK_EX);
    tcountfp=fdopen(tcountfd,"r+");
    threadcount=0;
  } else {
    flock(tcountfd,LOCK_EX);
    tcountfp=fdopen(tcountfd,"r+");
    fscanf(tcountfp,"%d",&threadcount);
  }
}

void updateandclosethreadcount()
{
  fseek(tcountfp,0,SEEK_SET);
  fprintf(tcountfp,"%d\n",threadcount);
  fclose(tcountfp);
  flock(tcountfd,LOCK_UN);
  close(tcountfd);
}

int getmessage(char **subject, char **name, char **message)
{
  char *inbuf;
  char **fields;
  char **data;
  int count;

  readCGIdata(&inbuf);
  extractCGIinput(inbuf,&fields,&data,&count);

  if((count!=3) || (strcmp(fields[0],"subject")!=0) ||
     (strcmp(fields[1],"name")!=0) || (strcmp(fields[2],"message")!=0)) {
    return -1;
  }

  if((strcmp(data[0],"")==0) || (strcmp(data[1],"")==0) || 
     (strcmp(data[2],"")==0)) {
    return -2;
  }

  decodeCGIstr(data[0]);
  decodeCGIstr(data[1]);
  decodeCGIstr(data[2]);
  
  *subject=HTMLencode(data[0]);
  *name=HTMLencode(data[1]);
  *message=HTMLencode(data[2]);

  free(data[0]);
  free(data[1]);
  free(data[2]);

  free(fields[0]);
  free(fields[1]);
  free(fields[2]);
  free(fields);
  free(data);
  return 1;
}

char *format2digit(char *buf, int n)
{
  /*  y+=1900+543; */
  n%=100;
  if(n==0)
    strcpy(buf,"00");
  else if(n<10)
    sprintf(buf,"0%d",n);
  else
    sprintf(buf,"%d",n);
  return buf;
}

void gettimeinfo(char *dstr, char *tstr)
{
  time_t timep;
  struct tm *timeptr;
  char yearbuf[10];
  char minbuf[10];

  char *monstr[]={"Jan","Feb","Mar","Apr","May","Jun",
		  "Jul","Aug","Sep","Oct","Nov","Dec"};

  timep=time(NULL);
  timeptr=localtime(&timep);
  sprintf(dstr,"%d %s %s", timeptr->tm_mday, monstr[timeptr->tm_mon],
	  format2digit(yearbuf,timeptr->tm_year));
  sprintf(tstr,"%d:%s", timeptr->tm_hour, 
	  format2digit(minbuf,timeptr->tm_min));
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

char *assemblyhtmldoc(char **htmldata, char *subject, char *name, char *message, 
		      char *datestr, char *timestr)
{
  char **flist, **vlist;
  char *messagebody;
  char messageinfo[100];

  createstrarray(&flist,6, "thread-subject", "message", 
		 "name", "date", "time",
		 "newmessage-placeholder");
  createstrarray(&vlist,6,subject,message,name,datestr,timestr,
		 NEWMESSAGEPLACEHOLDER);

  messagebody=putintemplate(buildfilename(BODYTEMPLATE,""), 6,
			    flist, vlist);
  free(flist);
  free(vlist);

  sprintf(messageinfo,MESSAGEINFOFORMAT,threadcount+1);

  createstrarray(&flist,2,"new-message","message-info");
  createstrarray(&vlist,2,messagebody, messageinfo);

  *htmldata=putintemplate(buildfilename(NEWMESSAGETEMPLATE,""), 2,
			  flist, vlist);

  return *htmldata;
}

void writehtmldoc(int threadid, char *htmldata)
{
  FILE *fp;
  char idstr[10];

  sprintf(idstr,"%d",threadid);
  if((fp=fopen(buildfilename(idstr,".html"),"w"))!=NULL) {
    fprintf(fp,"%s",htmldata);
    fclose(fp);
  }
}

void writethreadheader(int threadid, 
		       char *subject, char *name, 
		       char *datestr, char *timestr)
{
  FILE *fp;
  char idstr[10];

  sprintf(idstr,"%d",threadid);
  if((fp=fopen(buildfilename(idstr,".head"),"w"))!=NULL) {
    fprintf(fp,"%s\n",subject);
    fprintf(fp,"%s\n",name);
    fprintf(fp,"%s\n",datestr);
    fprintf(fp,"%s\n",timestr);
    fprintf(fp,"0\n");
    fclose(fp);
  }  
}

void printrefreshhtml(int threadid)
{
  char **flist, **vlist;
  char meta[100], tag[100];

  sprintf(meta,"<meta http-equiv=\"refresh\" content=\"10;URL=read.cgi?%d.html\">",
	  threadid);
  sprintf(tag,"<a href=\"read.cgi?%d.html\">",threadid);

  createstrarray(&flist,2, "msgrefreshmeta", "msglinktag");
  createstrarray(&vlist,2, meta, tag);

  printf("%s",putintemplate(buildfilename(REFRESHTEMPLATE,""),
			    2,flist,vlist));

  free(flist);
  free(vlist);
}

main()
{
  char *subject, *name, *message;
  char datestr[10], timestr[10];
  char *htmldata;

  initHTMLencodingtable();

  printf("Content-type: text/html\n");
  printf("\n");

  getthreadcount();

  if(getmessage(&subject, &name, &message)>0) {
    gettimeinfo(datestr,timestr);
    assemblyhtmldoc(&htmldata, subject, name, message, datestr, timestr);
    writehtmldoc(threadcount+1, htmldata);
    writethreadheader(threadcount+1, subject, name, datestr, timestr);

    threadcount++;

    printrefreshhtml(threadcount);
  } else {
    printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    printf("<html>\n");

    printf("wrong message format.<br>");

    printf("</html>\n");
  }

  updateandclosethreadcount();


  /* so many thing left to clear up... ignored... sorry */
}

