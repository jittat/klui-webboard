CC = gcc
CFLAGS =

all: webboard postmessage read adduser

clean:
	rm *.o
	rm webboard newmessage postmessage read adduser

depend:
	makedepend *.c

webboard: webboard.o cgitool.o strlist.o tmpl.o
	$(CC) $(CFLAGS) -o webboard webboard.o cgitool.o \
	strlist.o tmpl.o

postmessage: postmessage.o cgitool.o strlist.o tmpl.o html.o account.o macro.o
	$(CC) $(CFLAGS) -o postmessage postmessage.o cgitool.o \
	strlist.o tmpl.o html.o account.o macro.o
	chmod +s postmessage

read: read.o cgitool.o strlist.o
	$(CC) $(CFLAGS) -o read read.o cgitool.o strlist.o

dumpcgi: dumpcgi.o cgitool.o strlist.o
	$(CC) $(CFLAGS) -o dumpcgi dumpcgi.o cgitool.o strlist.o

adduser: adduser.o cgitool.o account.o tmpl.o strlist.o html.o
	$(CC) $(CFLAGS) -o adduser adduser.o cgitool.o strlist.o tmpl.o account.o \
	html.o
	chmod +s adduser

t: t.o cgitool.o
	$(CC) $(CFLAGS) -o t t.o cgitool.o strlist.o

test-tmpl: test-tmpl.o cgitool.o strlist.o tmpl.o
	$(CC) $(CFLAGS) -o test-tmpl test-tmpl.o cgitool.o strlist.o tmpl.o

webboard.o: webboard.c
postmessage.o: postmessage.c
newmessage.o: newmessage.c
read.o: read.c
adduser.o: adduser.c

dumpcgi.o: dumpcgi.c
t.o: t.c
test.o: test.c

html.o: html.c
cgitool.o: cgitool.c
strlist.o: strlist.c
tmpl.o: tmpl.c
account.o: account.c
macro.o: macro.c

#.o.c:
#	$(CC) $(CFLAGS) -c $<


# DO NOT DELETE


