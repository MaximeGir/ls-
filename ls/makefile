# variables

cc=gcc
options=-std=gnu99 -Wall
bin=whatSize
objets=util.o whatSize.o

#cible : dépendance 

whatSize: $(objets)
	$(cc) $(options) $(objets) -o $(bin)

util.o: util.h util.c
	$(cc) $(options) -c util.c -o util.o

whatSize.o: whatSize.c
	$(cc) $(options) -c whatSize.c -o whatSize.o

#cible : nettoyage
	
clean:
	\rm -f *.o
	\rm -f $(bin)
	\rm -f *~

#cible : archivage

archive:
	tar cvf whatSize.tar *
	gzip whatSize.tar
