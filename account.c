#include "account.h"
#include <stdio.h>
#include <sys/file.h>

#define  USERLIST_FILE  "userlist"

#define  USERINFO_FILE  "userinfo"
#define  USERTEMP_FILE  "newuserinfo"

static char *nextchar(char c,char *st)
{
  while((*st!=0) && (*st!=c))
    st++;
  return st;
}

static void extractline(char *line, char **name, char **passwd, 
                        char **dispname, char **pic)
{
  *name=line;
  line=nextchar(':',line);
  *line=0; line++;

  *passwd=line;
  line=nextchar(':',line);
  *line=0; line++;

  *dispname=line;
  line=nextchar(':',line);
  *line=0; line++;

  *pic=line;
  line=nextchar(':',line);
  *line=0; line++;
}

static int openandlock(char *fname, int *fd, FILE **fp, int lockop)
{
  (*fd)=open(fname,O_RDWR);
  if((*fd)!=-1) {
    flock(*fd, lockop);
    (*fp)=fdopen(*fd,"r+");
  }
  return *fd;
}

static int createandlock(char *fname, int *fd, FILE **fp, int lockop)
{
  (*fd)=open(fname,O_RDWR|O_CREAT);
  flock(*fd, lockop);
  (*fp)=fdopen(*fd,"r+");
  return *fd;
}

static void closeandrelease(int fd, FILE *fp)
{
  fclose(fp);
  flock(fd,LOCK_UN);
  close(fd);
}

int getuserinfo(char *name, char *passwd, char *dispname, char *pic)
{
  FILE *fp;
  int fd;
  char line[100];
  int len;
  char *lname, *lpasswd, *ldname, *lpic;

  openandlock(USERINFO_FILE,&fd,&fp,LOCK_EX);
  if(fp==NULL)
    return 0;

  while(fgets(line,100,fp)!=NULL) {
    len=strlen(line);
    if(len!=0)
      line[len-1]=0;

    if(*line!=0) {
      extractline(line,&lname,&lpasswd,&ldname,&lpic);
      if((strcmp(lname,name)==0) && (strcmp(lpasswd,passwd)==0)) {
        strcpy(dispname,ldname);
        strcpy(pic,lpic);
        closeandrelease(fd,fp);
        return 1;
      }
    }
  }
  closeandrelease(fd,fp);
  return 0;
}

int getuserinfowithoutpasswd(char *name, char *dispname, char *pic)
{
  FILE *fp;
  int fd;
  char line[100];
  int len;
  char *lname, *lpasswd, *ldname, *lpic;

  openandlock(USERINFO_FILE,&fd,&fp,LOCK_EX);
  if(fp==NULL)
    return 0;

  while(fgets(line,100,fp)!=NULL) {
    len=strlen(line);
    if(len!=0)
      line[len-1]=0;

    if(*line!=0) {
      extractline(line,&lname,&lpasswd,&ldname,&lpic);
      if(strcmp(lname,name)==0) {
        strcpy(dispname,ldname);
        strcpy(pic,lpic);
        closeandrelease(fd,fp);
        return 1;
      }
    }
  }
  closeandrelease(fd,fp);
  return 0;
}

static int testuser(char *fname, char *name, int ifname, 
                    char *passwd, int ifpasswd)
{
  FILE *fp;
  int fd;
  char line[100];
  int len;
  char *lname, *lpasswd, *ldname, *lpic;

  openandlock(fname,&fd,&fp,LOCK_EX);
  if(fp==NULL)
    return 0;

  while(fgets(line,100,fp)!=NULL) {
    len=strlen(line);
    if(len!=0)
      line[len-1]=0;

    if(*line!=0) {
      extractline(line,&lname,&lpasswd,&ldname,&lpic);
      if(((!ifname) || (strcmp(lname,name)==0)) &&
         ((!ifpasswd) || (strcmp(lpasswd,passwd)==0))) {
        closeandrelease(fd,fp);
        return 1;
      }
    }
  }
  closeandrelease(fd,fp);
  return 0;
}

int checkuser(char *name, char *passwd)
{
  return testuser(USERINFO_FILE, name, 1, passwd, 1);
}

int tempuserexist(char *name)
{
  return testuser(USERTEMP_FILE, name, 1, "", 0);
}

int userexist(char *name)
{
  return testuser(USERINFO_FILE, name, 1, "", 0);
}

static void adduser(char *fname, char *name, char *passwd, 
                    char *dispname, char *pic,
                    char *email, char *ref)
{
  FILE *fp;
  int fd;

  openandlock(fname, &fd, &fp, LOCK_EX);
  fseek(fp,0,SEEK_END);
  fprintf(fp,"%s:%s:%s:%s:%s:%s\n", name, passwd, dispname, pic, email, ref);
  closeandrelease(fd,fp);
}

void adduserinfo(char *name, char *passwd, char *dispname, char *pic,
                 char *email, char *ref)
{
  adduser(USERINFO_FILE, name, passwd, dispname, pic, email, ref);
}


void addtempuserinfo(char *name, char *passwd, char *dispname, char *pic,
                     char *email, char *ref)
{
  adduser(USERTEMP_FILE, name, passwd, dispname, pic, email, ref);
}


