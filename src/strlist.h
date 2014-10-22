#ifndef STRLIST_H_INCLUDED
#define STRLIST_H_INCLUDED

struct strlistnode;

struct strlist {
  struct strlistnode *front, *rear;
};

void newlist(struct strlist *list);
struct strlist *addtostrlist(struct strlist *list, char *st);
char **strlisttoarray(struct strlist *list);
char **strlisttonarray(struct strlist *list, int count);
char *strlisttostring(struct strlist *list);
void deletestrlist(struct strlist *list);

#endif

