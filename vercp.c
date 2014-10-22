#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>

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

main(int argc, char *argv[])
{
  FILE *fp;
  int fd;
  FILE *fin, *fold;
  char buff1[400],buff2[400];
  int checkpassed=1;

  if(argc!=4) {
    printf("using: vercp <current> <final> <old_current>\n");
    return 0; 
  }

  if((fin=fopen(argv[2],"r"))==NULL) {
    printf("cannot open source file.\n");
    return 0;
  }

  if((fold=fopen(argv[3],"r"))==NULL) {
    printf("cannot open source file.\n");
    return 0;
  }

  if(openandlock(argv[1],&fd,&fp,LOCK_EX)!=-1) {
    fseek(fp,0,SEEK_SET);
    printf("checking...(%s and %s)\n",argv[1],argv[3]);
    while(fgets(buff1,399,fp)!=NULL) {
      if((fgets(buff2,399,fold)==NULL) || (strcmp(buff1,buff2)!=0)) {
         printf("failed on integrity check.\n");
         checkpassed=0;
         break;
      }
    }
    if((checkpassed) && (fgets(buff2,399,fold)!=NULL))
      checkpassed=0;

    if(checkpassed) {
      printf("check passed\n");
      fseek(fp,0,SEEK_SET);
      while(fgets(buff1,399,fin)!=NULL)
        fprintf(fp,"%s",buff1);
      ftruncate(fd,ftell(fp));
    }

    fclose(fin);
    fclose(fold);
    closeandrelease(fd,fp);
  }
}

