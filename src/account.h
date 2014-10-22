#ifndef ACCOUNT_H_INCLUDED
#define ACCOUNT_H_INCLUDED

int getuserinfo(char *name, char *passwd, char *dispname, char *pic);
int getuserinfowithoutpasswd(char *name, char *dispname, char *pic);
int userexist(char *name);
int tempuserexist(char *name);
int checkuser(char *name, char *passwd);
void adduserinfo(char *name, char *passwd, char *dispname, char *pic,
                 char *email, char *ref);
void addtempuserinfo(char *name, char *passwd, char *dispname, char *pic,
                     char *email, char *ref);

#endif

