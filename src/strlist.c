#include "strlist.h"
#include <stdlib.h>
#include <string.h>

struct strlistnode {
  char *st;
  struct strlistnode *next;
};

void newstrlist(struct strlist *list)
{
  list->front=NULL;
  list->rear=NULL;
}

struct strlist *addtostrlist(struct strlist *list, char *st)
{
  struct strlistnode *temp;
  
  temp=(struct strlistnode *)malloc(sizeof(struct strlistnode));
  
  temp->st=st;
  temp->next=NULL;
  if(list->front==NULL)
    list->front=temp;
  if(list->rear!=NULL)
    (list->rear)->next=temp;
  list->rear=temp;
  
  return list;
}

char **strlisttonarray(struct strlist *list, int count)
{
   char **a=(char **)malloc(count*sizeof(char*));
   int i;
   struct strlistnode *node;

   i=0;
   node=list->front;
   while((node!=NULL) && (i<count)) {
      a[i]=node->st;
      node=node->next;
      i++;
   }
   for(; i<count; i++)
      a[i]=NULL;

   return a;
}

char **strlisttoarray(struct strlist *list)
{
  int count,i;
  struct strlistnode *node;
  char **a;

  count=0;
  node=list->front;
  while(node!=NULL) {
    count++;
    node=node->next;
  }
  
  a=(char **)malloc(count*sizeof(char*));
  
  i=0;
  node=list->front;
  while(node!=NULL) {
    a[i]=node->st;
    node=node->next;
    i++;
  }
  
  return a;
}

char *strlisttostring(struct strlist *list)
{
  int len;
  struct strlistnode *node;
  char *str, *p;

  len=0;
  node=list->front;
  while(node!=NULL) {
    len+=strlen(node->st);
    node=node->next;
  }
  
  str=(char *)malloc(len+1);
  
  p=str;
  node=list->front;
  while(node!=NULL) {
    strcpy(p,node->st);
    p+=strlen(node->st);
    node=node->next;
  }
  
  return str;
}

void deletestrlist(struct strlist *list)
{
  struct strlistnode *tmp, *node;

  node=list->front;
  while(node!=NULL) {
    tmp=node->next;
    free(node);
    node=tmp;
  }
}
