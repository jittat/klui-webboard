#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include "cgitool.h"
#include "strlist.h"
#include "tmpl.h"
#include "html.h"
#include "account.h"
#include "macro.h"

#define BOARDDIR                ""
#define THREAD_COUNT_FILE       "threadcount"

#define NEWBODY_TEMPLATE        "newmessagebody.template"
#define NEWMESSAGE_TEMPLATE     "newmessage.template"
#define POSTBODY_TEMPLATE       "postmessagebody.template"

#define PREVIEWNEW_TEMPLATE     "previewnewmessage.template"
#define PREVIEWMSG_TEMPLATE     "previewmessage.template"

#define GRAPHICS_DIR            "images/"
#define GRAPHICS_FILE_INPUT     "<input type=file name=\"graphics\" size=30>"
#define GRAPHICS_FILE_TAKEN     "[the picture was saved]"
#define GRAPHICS_INC_FORMAT     "<br><img src=\"%s\">"

#define OPTION_FORMAT           "<input name=\"option\" type=\"hidden\" value=\"%s\">"

#define NEWMESSAGE_PLACEHOLDER  "<!--template:new-message--!>"
#define MESSAGEINFO_FORMAT      "<input name=\"msgid\" type=\"hidden\" value=\"%d\">"
#define REFRESH_TEMPLATE        "refresh.template"

#define MAX_MESSAGE_LENGTH      200000
#define MSG_TOOLONG_TEMPLATE    "toolong.template"

#define WRONGFORMAT_TEMPLATE    "wrongformat.template"
#define WRONGPASSWORD_TEMPLATE    "wrongpasswd.template"

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

int openandlock(char *fname, int *fd, FILE **fp, int lockop)
{
  (*fd)=open(fname,O_RDWR);
  if((*fd)!=-1) {
    flock(*fd, lockop);
    (*fp)=fdopen(*fd,"r+");
  }
  return *fd;
}

int createandlock(char *fname, int *fd, FILE **fp, int lockop)
{
  (*fd)=open(fname,O_RDWR|O_CREAT);
  flock(*fd, lockop);
  (*fp)=fdopen(*fd,"r+");
  return *fd;
}

void closeandrelease(int fd, FILE *fp)
{
  fclose(fp);
  flock(fd,LOCK_UN);
  close(fd);
}

void getandlockthreadcount(int *tcountfd, FILE **tcountfp, int *threadcount)
{
  if(openandlock(buildfilename(THREAD_COUNT_FILE,""),
		 tcountfd,tcountfp,LOCK_EX)!=-1)
    fscanf(*tcountfp,"%d",threadcount);
  else {
    createandlock(buildfilename(THREAD_COUNT_FILE,""),
		  tcountfd, tcountfp, LOCK_EX);
    (*threadcount)=0;
  }
  if(*tcountfd==-1)
    printf("can't get threadcount<br>\n");
}

void updateandclosethreadcount(int tcountfd, FILE *tcountfp, int threadcount)
{
  fseek(tcountfp,0,SEEK_SET);
  fprintf(tcountfp,"%d\n",threadcount);
  closeandrelease(tcountfd,tcountfp);
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

char *processmessage(char *message)
{
  char *pmsg=expandmacro(message);
  char *encmsg=HTMLencode(pmsg);

  free(pmsg);
  return encmsg;
}

char *buildgraphicsincltag(char *tag, char *grpname)
{
  if(grpname==NULL) {
    *tag=0;
    return tag;
  } else {
    sprintf(tag,GRAPHICS_INC_FORMAT,grpname);
    return tag;
  }
}

/*
char *getdisplayname(char *name)
{
  char *p;
  char dispname[30];
  char imgfile[30];
  char formatedname[100];

  p=name;
  while((*p!=0) && (*p!='/'))
    p++;
  if(*p=='/') {
    *p=0;
    p++;
    if(getuserinfo(name,p,dispname,imgfile)) {
      p--;
      *p='/';
      return strdup(dispname);
    } else {
      p--;
      *p='/';
      return strdup(name);
    }
  } else
    return strdup(name);
}
*/

char *buildformatedname(char *name, char *dname)
{
  char *p;
  char dispname[50];
  char imgfile[30];
  char formatedname[100];

  if(dname[0]=='t') {
    if(dname[2]!=0)
      return HTMLencode(dname+2);  
    else {
      getuserinfowithoutpasswd(name,dispname,imgfile);
      if(*imgfile==0)
	return(HTMLencode(dispname));
      else {
	sprintf(formatedname,"%s <img src=\"userpics/%s\">",
		HTMLencode(dispname),imgfile);
	return strdup(formatedname);
      }
    }
  } else {
    sprintf(formatedname,"<img src=\"userpics/emo%c.gif\">",dname[0]);
    return strdup(formatedname);
  }
}

char *buildnewmessagebody(char *subject, char *name, char *dname,
			  char *message, char *datestr, char *timestr,
			  char *grpname)
{
  char **flist, **vlist;
  char *hsubject, *hname, *hmessage;
  char *body;
  char imagetag[100];

  hsubject=HTMLencode(subject);
  hname=buildformatedname(name, dname);
  hmessage=processmessage(message);

  buildgraphicsincltag(imagetag,grpname);

  createstrarray(&flist,7, "thread-subject", "message", "image",
		 "name", "date", "time",
		 "newmessage-placeholder");
  createstrarray(&vlist,7,hsubject,hmessage, imagetag,
		 hname,datestr,timestr,
		 NEWMESSAGE_PLACEHOLDER);

  body=putintemplate(buildfilename(NEWBODY_TEMPLATE,""), 7,
		     flist, vlist);
  free(flist);
  free(vlist);

  free(hsubject);
  free(hname);
  free(hmessage);

  return body;
}

char *buildmessagebody(char *name, char *dname, char *message,
		       char *datestr, char *timestr,
		       char *grpname)
{
  char **flist, **vlist;
  char *hname, *hmessage;
  char *body;
  char imagetag[100];

  hname=buildformatedname(name, dname);
  hmessage=processmessage(message);

  buildgraphicsincltag(imagetag,grpname);

  createstrarray(&flist,6, "message", "image",
		 "name", "date", "time",
		 "newmessage-placeholder");
  createstrarray(&vlist,6,hmessage,imagetag,hname,
		 datestr,timestr,
		 NEWMESSAGE_PLACEHOLDER);

  body=putintemplate(buildfilename(POSTBODY_TEMPLATE,""), 6,
			    flist, vlist);
  free(flist);
  free(vlist);

  free(hname);
  free(hmessage);

  return body;
}

char *assemblynewmessage(char **htmldata, int threadid, 
			 char *subject, char *name, char *dname,
                         char *message, 
			 char *datestr, char *timestr, char *grpname)
{
  char **flist, **vlist;
  char *messagebody;
  char messageinfo[100];

  messagebody=buildnewmessagebody(subject, name, dname,
				  message, datestr, timestr, 
				  grpname);
  
  sprintf(messageinfo,MESSAGEINFO_FORMAT,threadid);

  createstrarray(&flist,2,"new-message","message-info");
  createstrarray(&vlist,2,messagebody, messageinfo);

  *htmldata=putintemplate(buildfilename(NEWMESSAGE_TEMPLATE,""), 2,
			  flist, vlist);

  free(messagebody);

  return *htmldata;
}

char *assemblypostmessage(char **htmldata, FILE *oldhtml,
			  int threadid,  
			  char *name, char *dname, char *message, 
			  char *datestr, char *timestr, char *grpname)
{
  char **flist, **vlist;
  char *messagebody;
  char messagetemplate[100];

  messagebody=buildmessagebody(name, dname, message, 
			       datestr, timestr, grpname);

  sprintf(messagetemplate,"%d.html",threadid);

  createstrarray(&flist,1,"new-message");
  createstrarray(&vlist,1,messagebody);

  *htmldata=putinftemplate(oldhtml, 1, flist, vlist);

  free(messagebody);

  return *htmldata;
}

void loguser(int threadid, int postid,
             char *name, char *datestr, char *timestr)
{
  FILE *fp;
  char fname[20];

  sprintf(fname,"%d.log",threadid);
  if((fp=fopen(fname,"a"))==NULL)
    fp=fopen(fname,"w");
  if(fp==NULL)
    return;
  fprintf(fp,"%d/%s/%s/%s\n",postid,datestr,timestr,name);
  fclose(fp);
}

void createthreadheader(int threadid, 
			char *subject, char *name, char *dname,
			char *datestr, char *timestr)
{
  FILE *fp;
  char idstr[10];
  char *dispname;

  sprintf(idstr,"%d",threadid);
  if((fp=fopen(buildfilename(idstr,".head"),"w"))!=NULL) {
    fprintf(fp,"%s\n",subject);
    dispname=buildformatedname(name,dname);
    fprintf(fp,"%s\n",dispname);
    free(dispname);
    fprintf(fp,"%s\n",datestr);
    fprintf(fp,"%s\n",timestr);
    fprintf(fp,"0\n");
    fclose(fp);

    loguser(threadid,1,name,datestr,timestr);
  }  
}

void updatethreadheader(int threadid, char *name, char *dname,
                        char *datestr, char *timestr)
{
  char hname[10];
  int i, postcount;
  int fd;
  FILE *fp;
  char line[200];
  char *dispname;

  sprintf(hname,"%d.head",threadid);
  fd=open(buildfilename(hname,""),O_RDWR);
  if(fd==-1)
    return;
  flock(fd,LOCK_EX);
  fp=fdopen(fd,"r+");
  for(i=0; i<4; i++)
    fgets(line,200,fp);
  fgets(line,200,fp);
  sscanf(line,"%d",&postcount);
  fseek(fp,0,SEEK_SET);
  for(i=0; i<4; i++)
    fgets(line,200,fp);
  postcount++;
  fprintf(fp,"%d\n",postcount);
  dispname=buildformatedname(name,dname);
  fprintf(fp,"%s\n%s\n%s\n",dispname,datestr,timestr);
  free(dispname);
  fclose(fp);

  loguser(threadid,postcount,name,datestr,timestr);

  flock(fd,LOCK_UN);
  close(fd);
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

void printrefreshhtml(int threadid)
{
  char **flist, **vlist;
  char meta[100], tag[100];

  sprintf(meta,"<meta http-equiv=\"refresh\" content=\"10;URL=read.cgi?%d.html\">",
	  threadid);
  sprintf(tag,"<a href=\"read.cgi?%d.html\">",threadid);

  createstrarray(&flist,2, "msgrefreshmeta", "msglinktag");
  createstrarray(&vlist,2, meta, tag);

  printf("%s",putintemplate(buildfilename(REFRESH_TEMPLATE,""),
			    2,flist,vlist));

  free(flist);
  free(vlist);
}

void printmessagefromtemplate(char *templatename)
{
  printf("%s",putintemplate(buildfilename(templatename,""),
			    0,NULL,NULL));
}

formfield *findfield(char *index, formfield *fields, int count)
{
  int i;

  i=0;
  while((i<count) && (strcmp(fields[i].name,index)!=0))
    i++;
  if(i<count)
    return &(fields[i]);
  else
    return NULL;
}

char *getimagefilefromoptionst(char *st)
{
  char *mark="graphics='";
  char *p,*q,*fname;
  int len;

  if(st==NULL)
    return NULL;

  if((p=strstr(st,mark))!=0) {
    p+=strlen(mark);
    q=p;
    while((*q!=0) && (*q!='\''))
      q++;
    len=q-p;
    fname=(char *)malloc(len+1);
    strncpy(fname,p,len);
    fname[len]=0;
    return fname;
  } else
    return NULL;
}

char *getimagefilefromoption(formfield *option)
{
  if(option==NULL)
    return NULL;
  else
    return getimagefilefromoptionst(option->data);
}

char *getdname(char *dname, formfield *fields, int count)
{
  formfield *dnamefield, *facefield;

  dnamefield=findfield("dname", fields, count);
  facefield=findfield("face", fields, count);

  if((dnamefield==NULL)||(facefield==NULL))
    sprintf(dname,"t:");
  else
    sprintf(dname,"%s:%s",facefield->data,dnamefield->data);
}

char *builddnamehtml(char *dname)
{
  struct strlist dnamehtml;
  char buffer[500];
  int i;
  char *outstring;

  newstrlist(&dnamehtml);

  if(dname[0]=='t')
    addtostrlist(&dnamehtml, 
		 strdup("<input id=\"facetext\" name=\"face\" type=\"radio\" value=\"t\" checked>\n"));
  else
    addtostrlist(&dnamehtml, 
		 strdup("<input id=\"facetext\" name=\"face\" type=\"radio\" value=\"t\">\n"));
  
  sprintf(buffer,
	  "<input name=\"dname\" type=\"text\" size=20 maxlength=60 onFocus=\"return document.forms[0].facetext.checked=true\" value=\"%s\"> <br>\n",
	  dname+2);
  addtostrlist(&dnamehtml,strdup(buffer));

  for(i=1; i<=8; i++) {   // 8 emotional faces
    if(dname[0]==i+'0')
      sprintf(buffer,"<input name=\"face\" type=\"radio\" value=\"%d\" checked><img src=\"userpics/emo%d.gif\">",i,i);
    else
      sprintf(buffer,"<input name=\"face\" type=\"radio\" value=\"%d\"><img src=\"userpics/emo%d.gif\">",i,i);
    addtostrlist(&dnamehtml,strdup(buffer));
  }
  
  outstring=strlisttostring(&dnamehtml);
  deletestrlist(&dnamehtml);

  return outstring;
}

void postnewmessage(formfield *fields, int count)
{
  char *subject, *name, *passwd, *message;
  char dname[100];
  formfield *subjfield, *namefield, *passwdfield, *msgfield;
  char datestr[10], timestr[10];
  char *htmldata;
  int threadid, tcountfd, threadcount;
  FILE *tcountfp;

  getandlockthreadcount(&tcountfd, &tcountfp, &threadcount);

  threadid=threadcount+1;

  subjfield=findfield("subject",fields,count);
  namefield=findfield("name",fields,count);
  getdname(dname,fields,count); 
  passwdfield=findfield("password",fields,count);
  msgfield=findfield("message",fields,count);

  if((subjfield==NULL) || (namefield==NULL) || (msgfield==NULL)
     || (passwdfield==NULL))
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
  else {
    subject=subjfield->data;
    name=namefield->data;
    passwd=passwdfield->data;
    message=msgfield->data;

    if(!checkuser(name,passwd))
      printmessagefromtemplate(WRONGPASSWORD_TEMPLATE);
    else if((*subject==0) || (*name==0))
      printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
    else {

      gettimeinfo(datestr,timestr);
      assemblynewmessage(&htmldata, threadid, subject, name, dname, 
			 message, datestr, timestr,
			 getimagefilefromoption(findfield("option",
							  fields,count)));

      writehtmldoc(threadid, htmldata);
      createthreadheader(threadid, subject, name, dname, datestr, timestr);
      
      threadcount++;
      
      printrefreshhtml(threadcount);
    }
  }

  updateandclosethreadcount(tcountfd, tcountfp, threadcount);
}

void postmessage(formfield *fields, int count)
{
  char *msgid, *name, *message, *passwd;
  char dname[100];
  formfield *msgidfield, *namefield, *passwdfield, *msgfield;
  int threadid;
  char datestr[10], timestr[10];
  char htmlfname[100];
  char *htmldata;

  int msgfd;
  FILE *msgfp;

  msgidfield=findfield("msgid",fields,count);
  namefield=findfield("name",fields,count);
  getdname(dname, fields, count);
  passwdfield=findfield("password",fields,count);
  msgfield=findfield("message",fields,count);

  if((msgidfield==NULL) || (namefield==NULL) || (msgfield==NULL)
     || (passwdfield==NULL))
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
  else {
    
    msgid=msgidfield->data;
    name=namefield->data;
    passwd=passwdfield->data;
    message=msgfield->data;

    if(!checkuser(name,passwd))
      printmessagefromtemplate(WRONGPASSWORD_TEMPLATE);
    else if((*msgid==0) || (*name==0))
      printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
    else {
      
      threadid=atoi(msgid);
      
      gettimeinfo(datestr,timestr);
      
      sprintf(htmlfname,"%d.html",threadid);
      
      openandlock(buildfilename(htmlfname,""),
		  &msgfd, &msgfp, LOCK_EX);
      
      if(msgfd==-1) {
	printf("error!!\n");
	return;
      }
      
      assemblypostmessage(&htmldata, msgfp, 
			  threadid, name, dname, message, datestr, timestr,
			  getimagefilefromoption(findfield("option",
							   fields,count)));
      
      fseek(msgfp,0,SEEK_SET);
      
      fprintf(msgfp,"%s", htmldata);
      
      updatethreadheader(threadid, name, dname, datestr, timestr);

      closeandrelease(msgfd, msgfp);
      
      printrefreshhtml(threadid);
    }
  }
}

char *putintoformvalue(char *st)
{
  char *valuetag="value=\"";
  char *value;

  value=(char *)malloc(strlen(st)+strlen(valuetag)+3);
  sprintf(value,"%s%s\"",valuetag,st);

  return value;
}

char *assemblypreviewnewmsg(char *prevmessage, char *subject,
			    char *name, char *dname, char *passwd,
			    char *message, char *option)
{
  char **flist, **vlist;

  char *hsubject, *hname, *hpasswd, *tmsg, *hmessage;
  char *subvalue, *namevalue, *passwdvalue, *dnamehtml;

  char optiontag[100];
  char *grpinputtag;

  char *prvmsg;

  hsubject=HTMLencode(subject);
  subvalue=putintoformvalue(hsubject);

  hname=HTMLencode(name);
  namevalue=putintoformvalue(hname);

  hpasswd=HTMLencode(passwd);
  passwdvalue=putintoformvalue(hpasswd);

  dnamehtml=builddnamehtml(dname);

  tmsg=expandbackslash(message);
  hmessage=HTMLencode_exclude_br(tmsg);

  sprintf(optiontag,OPTION_FORMAT,option);

  if(getimagefilefromoptionst(option)==NULL)
    grpinputtag=GRAPHICS_FILE_INPUT;
  else
    grpinputtag=GRAPHICS_FILE_TAKEN;

  createstrarray(&flist,8, 
		 "previewmessage", "optiontag", "subjectvalue",
		 "namevalue", "passwdvalue", 
                 "dnamehtml", "messagevalue", "grpinputtag");
  createstrarray(&vlist,8, 
		 prevmessage, optiontag, subvalue,
		 namevalue, passwdvalue, 
                 dnamehtml, hmessage, grpinputtag);

  prvmsg=putintemplate(buildfilename(PREVIEWNEW_TEMPLATE,""), 7,
		       flist, vlist);

  free(flist);
  free(vlist);

  free(tmsg);
  free(hmessage);
  free(namevalue);
  free(subvalue);
  free(hname);
  free(hsubject);
  free(hpasswd);
  free(passwdvalue);

  return prvmsg;
}

void previewnewmessage(formfield *fields, int count)
{
  char *subject, *name, *passwd, *message, *optst;
  char dname[100];
  formfield *subjfield, *namefield, *passwdfield, *msgfield, *option;
  char datestr[10], timestr[10];
  char *messagebody, *prvmsg;
  char *grpfname;

  subjfield=findfield("subject",fields,count);
  namefield=findfield("name",fields,count);
  passwdfield=findfield("password",fields,count);
  getdname(dname, fields, count);
  msgfield=findfield("message",fields,count);
  option=findfield("option", fields, count);

  grpfname=getimagefilefromoption(option);
  if(option!=NULL)
    optst=option->data;
  else
    optst="";

  if((subjfield==NULL) || (namefield==NULL) || (msgfield==NULL)
     || (passwdfield==NULL)) 
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
  else {
    subject=subjfield->data;
    name=namefield->data;
    passwd=passwdfield->data;
    message=msgfield->data;

    if((*subject==0) || (*name==0))
      printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
    else if(!checkuser(name,passwd)) 
      printmessagefromtemplate(WRONGPASSWORD_TEMPLATE);
    else {

      gettimeinfo(datestr,timestr);

      messagebody=buildnewmessagebody(subject, name, dname, message, 
				      datestr, timestr,
				      grpfname);

      prvmsg=assemblypreviewnewmsg(messagebody, 
				   subject, name, dname, passwd, message, 
				   optst);

      printf("%s",prvmsg);

      free(prvmsg);
      free(messagebody);
    }
  }
}

char *assemblypreviewmsg(char *prevmessage, char *threadid, 
			 char *name, char *dname, char *passwd,
                         char *message, char *option)
{
  char **flist, **vlist;

  char *hname, *hpasswd, *tmsg, *hmessage, threadidtag[100];

  char *namevalue, *passwdvalue;
  char *dnamehtml;

  char optiontag[100];
  char *grpinputtag;

  char *prvmsg;

  sprintf(threadidtag,"<input name=\"msgid\" type=\"hidden\" value=\"%s\">",
	  threadid);

  hname=HTMLencode(name);
  namevalue=putintoformvalue(hname);

  hpasswd=HTMLencode(passwd);
  passwdvalue=putintoformvalue(hpasswd);

  dnamehtml=builddnamehtml(dname);

  tmsg=expandbackslash(message);
  hmessage=HTMLencode_exclude_br(tmsg);

  sprintf(optiontag,OPTION_FORMAT,option);
  
  if(getimagefilefromoptionst(option)==NULL)
    grpinputtag=GRAPHICS_FILE_INPUT;
  else
    grpinputtag=GRAPHICS_FILE_TAKEN;
  
  createstrarray(&flist,8, 
		 "previewmessage", "threadidtag", "optiontag",
		 "namevalue", "dnamehtml", "passwdvalue",
                 "messagevalue", "grpinputtag");
  createstrarray(&vlist,8, 
		 prevmessage, threadidtag, optiontag,
		 namevalue, dnamehtml, passwdvalue,
                 hmessage, grpinputtag);

  prvmsg=putintemplate(buildfilename(PREVIEWMSG_TEMPLATE,""), 8,
		       flist, vlist);

  free(flist);
  free(vlist);

  free(tmsg);
  free(hmessage);
  free(namevalue);
  free(passwdvalue);
  free(dnamehtml);
  free(hname);
  free(hpasswd);

  return prvmsg;
}

void previewmessage(formfield *fields, int count)
{
  char *threadid, *name, *passwd, *message;
  char dname[100];

  formfield *idfield, *namefield, *passwdfield, *msgfield, *option;
  char datestr[10], timestr[10];
  char *messagebody, *prvmsg;
  char *grpfname, *optst;

  idfield=findfield("msgid",fields,count);
  namefield=findfield("name",fields,count);
  passwdfield=findfield("password",fields,count);
  getdname(dname,fields,count);

  msgfield=findfield("message",fields,count);
  option=findfield("option", fields, count);

  grpfname=getimagefilefromoption(option);
  if(option!=NULL)
    optst=option->data;
  else
    optst="";

  if((idfield==NULL) || (namefield==NULL) || (msgfield==NULL) ||
     (passwdfield==NULL))
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
  else {
    threadid=idfield->data;
    name=namefield->data;
    passwd=passwdfield->data;
    message=msgfield->data;

    if((*threadid==0) || (*name==0))
      printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
    if(!checkuser(name,passwd))
      printmessagefromtemplate(WRONGPASSWORD_TEMPLATE);
    else {

      gettimeinfo(datestr,timestr);

      messagebody=buildmessagebody(name, dname, message, 
				   datestr, timestr,
				   grpfname);

      prvmsg=assemblypreviewmsg(messagebody, 
				threadid, name, dname, passwd, message,
				optst);

      printf("%s",prvmsg);

      free(prvmsg);
      free(messagebody);
    }
  }
}

char *getimagefilename(char *fname, char *ext)
{
  char *countfname=buildfilename(GRAPHICS_DIR,"count");
  int fd;
  FILE *fp;
  int count;

  openandlock(countfname,&fd, &fp, LOCK_EX);
  fscanf(fp,"%d",&count);
  fseek(fp,0,SEEK_SET);
  count++;
  fprintf(fp,"%d\n",count);
  closeandrelease(fd,fp);

  sprintf(fname,"%s%d.%s",GRAPHICS_DIR,count,ext);
  return fname;
}

char *getext(formfield *grp, char *ext, int len)
{
  char *p, *q;
  int count;

  if(grp->attr==NULL) {
    *ext=0;
    return ext;
  }

  p=grp->attr;
  do {
    q=p+1;
    while((*q!=0) && (*q!='.'))
      q++;
    if(*q!=0)
      p=q;
  } while(*q!=0);

  if(*p!='.') {
    *ext=0;
    return ext;
  }

  count=0;
  p++;
  q=ext;
  while((*p!='\"') && (count<len)) {
    *q=tolower(*p);
    q++;
    p++;
    count++;
  }
  *q=0;

  return ext;
}

char *savegraphics(formfield *grp)
{
  char ext[10];
  char fname[30];
  FILE *fp;

  getext(grp,ext,10);

  if((strcmp(ext,"jpg")!=0) && (strcmp(ext,"gif")!=0))
    return NULL;

  getimagefilename(fname,ext);

  fp=fopen(fname,"wb");
  if(fp!=NULL) {
    fwrite(grp->data,grp->len,1,fp);
    fclose(fp);
    return strdup(fname);
  } else
    return NULL;
}

void processpreviewrequest(formfield *fields, int count)
{
  formfield *command;

  command=findfield("command", fields, count);

  if(command==NULL)
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
  
  else if(strcmp(command->data,"postnew")==0)
    previewnewmessage(fields,count);
  
  else if(strcmp(command->data,"post")==0) 
    previewmessage(fields,count);
  
  else
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
}

void processpostrequest(formfield *fields, int count)
{
  formfield *command;

  command=findfield("command", fields, count);

  if(command==NULL)
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
  
  else if(strcmp(command->data,"postnew")==0)
    postnewmessage(fields,count);
  
  else if(strcmp(command->data,"post")==0) 
    postmessage(fields,count);
  
  else
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
}

void processrequest(formfield *fields, int count)
{
  formfield *submittype;
  formfield *graphics;
  formfield *option;
  char *gfname;
  char newoption[20];

  graphics=findfield("graphics", fields, count);
  if((graphics!=NULL) && (graphics->len!=0)) {
    gfname=savegraphics(graphics);
    if(gfname!=NULL) {
      option=findfield("option",fields, count);

      if(option!=NULL) {
	sprintf(newoption,"graphics='%s'; %s",gfname,option->data);
	free(option->data);
	free(gfname);
	option->data=strdup(newoption);
      }
    }
  }

  submittype=findfield("submit", fields, count);
  
  if(submittype==NULL) 
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);

  else if(strcmp(submittype->data,"post message")==0)
    processpostrequest(fields, count);

  else if(strcmp(submittype->data,"preview")==0) 
    processpreviewrequest(fields, count);

  else
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
}

void clearfields(formfield *fields, int count)
{
  int i;

  for(i=0; i<count; i++) {
    free(fields[i].name);
    free(fields[i].data);
  }
}

main()
{
  formfield *fields;
  formfield *submittype;
  int fieldcount;

  char *lenstr;

  printf("Content-type: text/html\n");
  printf("\n");

  initHTMLencodingtable();

  lenstr=getenv("CONTENT_LENGTH");

  if((lenstr==NULL) || (atoi(lenstr)>MAX_MESSAGE_LENGTH)) {
    printmessagefromtemplate(MSG_TOOLONG_TEMPLATE);
    return;
  } 

  extractCGIinput(&fields,20,&fieldcount);

  processrequest(fields,fieldcount);

  clearfields(fields,fieldcount);
  free(fields);
}
