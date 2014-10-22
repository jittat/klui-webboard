#ifndef CGITOOL_H_INCLUDED
#define CGITOOL_H_INCLUDED

typedef struct {
  char *name;
  char *attr;
  int len;
  unsigned char *data;
} formfield;

void readCGIdata(unsigned char **buffer, int *len);
void extractCGIinput(formfield **fields, int maxcount, int *count);

#endif

