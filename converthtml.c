#include <stdio.h>

main(int argc, char *argv[])
{
  FILE *fin, *fout;
  char buff[400];

  fin=fopen(argv[1],"r");
  fout=fopen(argv[2],"w");

  while((fgets(buff,399,fin))!=NULL) {
    if(strcmp(buff,"<tr><td align=\"right\"> Name: </td>\n")==0) {
      fgets(buff,399,fin);
         
      fprintf(fout,"<tr><td align=\"right\"> Username: </td>\n");
      fprintf(fout,"<td><input name=\"name\" type=\"text\" size=20 maxlength=60> </td></tr>\n");
      fprintf(fout,"\n");
      fprintf(fout,"<tr><td align=\"right\"> Password: </td>\n");
      fprintf(fout,"<td><input name=\"password\" type=\"password\" size=20 maxlength=60> </td></tr>\n");
      fprintf(fout,"\n");
      fprintf(fout,"<tr><td align=\"right\" nowrap> Display name: </td>\n");
      fprintf(fout,"<td>\n");
      fprintf(fout,"<input id=\"facetext\" name=\"face\" type=\"radio\" value=\"t\" checked>\n");
      fprintf(fout,"<input name=\"dname\" type=\"text\" size=20 maxlength=60 onFocus=\"return document.forms[0].facetext.checked=true\"> <br>\n");
      fprintf(fout,"<input name=\"face\" type=\"radio\" value=\"1\"><img src=\"userpics/emo1.gif\">\n");
      fprintf(fout,"<input name=\"face\" type=\"radio\" value=\"2\"><img src=\"userpics/emo2.gif\">\n");
      fprintf(fout,"<input name=\"face\" type=\"radio\" value=\"3\"><img src=\"userpics/emo3.gif\">\n");
      fprintf(fout,"<input name=\"face\" type=\"radio\" value=\"4\"><img src=\"userpics/emo4.gif\">\n");
      fprintf(fout,"</td>\n");
      fprintf(fout,"<td>\n");
      fprintf(fout,"<font size=\"0\">[leave blank to use the display name as you registered]\n");
      fprintf(fout,"</font></td></tr>\n");
      fprintf(fout,"\n");
      fprintf(fout,"<tr><td><br></td></tr>\n");
    } else
      fprintf(fout,"%s",buff);
  }
  fclose(fin);
  fclose(fout);
}

