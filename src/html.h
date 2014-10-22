#ifndef HTML_H_INCLUDED
#define HTML_H_INCLUDED

void initHTMLencodingtable();
char *HTMLencode(char *str);
char *HTMLencode_exclude_br(char *str);

#endif
