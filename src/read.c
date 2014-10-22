#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

main(int argc, char *argv[])
{
  int fd;
  FILE *fp;
  char line[100];
  char fname[100];
  char *p;

  printf("Content-type: text/html\n");
  printf("\n");

  strncpy(fname,argv[1],100);
  fname[99]=0;

  p=fname;
  while((*p!=0) && (*p!='\\'))
    p++;
  *p=0;

  fd=open(fname,O_RDONLY);
  if(fd==-1) {
    printf("%s\n",fname);
    printf("internal error\n");
    return;
  }
  flock(fd,LOCK_SH);
  fp=fdopen(fd,"r");

  while(fgets(line,100,fp)!=NULL)
    printf("%s",line);

  fclose(fp);
  flock(fd,LOCK_UN);
  close(fd);
}


