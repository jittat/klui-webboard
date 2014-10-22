#include <stdio.h>
#include <stdlib.h>
#include "cgitool.h"
#include "tmpl.h"
#include "account.h"

#define  BOARDDIR                ""
#define  MAX_MESSAGE_LENGTH   2000

#define  WRONG_PASSWORD_TEMPLATE    "wrongpasswd.template"
#define  MSG_TOOLONG_TEMPLATE       "toolong.template"
#define  WRONGFORMAT_TEMPLATE      "wrongformat.template"


static char tempfilename[100];

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

char *buildfilename(char *fname, char *ext)
{
  strcpy(tempfilename, BOARDDIR);
  strcat(tempfilename, fname);
  strcat(tempfilename, ext);

  return tempfilename;
}

void printmessagefromtemplate(char *templatename)
{
  printf("%s",putintemplate(buildfilename(templatename,""),
                            0,NULL,NULL));
}

char *filterstring(char *st)
{
  char *p=st;

  while(*p!=0) {
    if(*p==':')
      *p='#';
    p++;
  }
}

int nochar(char *st, char ch)
{
  char *p=st;

  while(*p!=0) {
    if(*p==ch)
      return 0;
    p++;
  }
  return 1;
}

int verifyinfo(char *name, char *passwd1, char *passwd2, char *dispname,
               char *email, char *ref)
{
  if((!nochar(name,' ')) || (!nochar(name,':'))) {
    printf("no spaces or colons in the username please. . . please try again\n");
    return 0;
  }
  if(strcmp(passwd1, passwd2)!=0) {
    printf("please verify your password. . . please try again\n");
    return 0;
  }
  if(!nochar(passwd1,':')) {
    printf("no colon in the password please. . . please try again\n");
    return 0;
  }
  if(!nochar(dispname,':')) {
    printf("no colon in the display name please. . . please try again\n");
    return 0;
  }
  if((!nochar(email,' ')) || (!nochar(email,':'))) {
    printf("no spaces or colons in the email please. . . please try again\n");
    return 0;
  }
  return 1;
}

void addnewuser(formfield *fields, int count)
{
  formfield *name, *passwd1, *passwd2, *dispname, *ref, *email;

  name=findfield("name",fields,count);
  passwd1=findfield("passwd1",fields,count);
  passwd2=findfield("passwd2",fields,count);
  dispname=findfield("dispname",fields,count);
  email=findfield("email",fields,count);
  ref=findfield("ref",fields,count);

  if((name==NULL) || (passwd1==NULL) || (passwd2==NULL) ||
     (dispname==NULL) || (email==NULL) || (ref==NULL)) {
    printf("wrong format\n");
    return;
  }

  if(userexist(name->data) || tempuserexist(name->data)) {
    printf("the username was taken. . . please try again\n");
    return;
  }

  if(verifyinfo(name->data, passwd1->data, passwd2->data, dispname->data, 
                email->data, ref->data)) {
    filterstring(ref->data);
    addtempuserinfo(name->data, passwd1->data, dispname->data, "", 
                    email->data, ref->data);
    printf("your account information has been received.  you will get an email after it is activated, so please wait.  this process should take a while.  so if you want a faster activation, do not be afraid to send us an e-mail.  <a href=\"index.html\">[return to webboard]</a>\n");
  }
}

void addchild(formfield *fields, int count)
{
  formfield *pname, *ppasswd, *name, *passwd1, *passwd2, 
            *dispname, *email, *ref;
  char pdispname[30];
  char pimgfile[30];
  char sref[100];

  pname=findfield("pname",fields,count);
  ppasswd=findfield("ppasswd",fields,count);
  name=findfield("name",fields,count);
  passwd1=findfield("passwd1",fields,count);
  passwd2=findfield("passwd2",fields,count);
  dispname=findfield("dispname",fields,count);
  email=findfield("email",fields,count);
  ref=findfield("ref",fields,count);

  if((pname==NULL) || (ppasswd==NULL) ||
     (name==NULL) || (passwd1==NULL) || (passwd2==NULL) ||
     (dispname==NULL) || (email==NULL) || (ref==NULL)) {
    printf("wrong format\n");
    return;
  }

  if(!getuserinfo(pname->data, ppasswd->data, pdispname, pimgfile)) {
    printf("invalid parent account. . . (check username and password)\n");
    return;
  }

  if(userexist(name->data)) {
    printf("the username was taken. . . please try again\n");
    return;
  }

  if(verifyinfo(name->data, passwd1->data, passwd2->data, 
                dispname->data, email->data, pname->data)) {
    sprintf(sref,"%s(invt=%s)",ref->data,pname->data);
    adduserinfo(name->data, passwd1->data, dispname->data, "", email->data, sref);
    printf("your account has been added to the system under the invitation of %s.  it is activated and you can use it now. <a href=\"index.html\">[return to webboard]</a>\n", pdispname);
  }
}

void processrequest(formfield *fields, int count)
{
  formfield *command;

  command=findfield("command",fields,count);

  if(command==NULL)
    printmessagefromtemplate(WRONGFORMAT_TEMPLATE);
  else if(strcmp(command->data,"addnew")==0)
    addnewuser(fields,count);
  else if(strcmp(command->data,"addchild")==0)
    addchild(fields,count);
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

