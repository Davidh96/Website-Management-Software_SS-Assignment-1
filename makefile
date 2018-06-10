objects = assignment.o AccessControl.o auditSite.o update.o backup.o
headers = AccessControl.h auditSite.h update.h backup.h
myprog: $(objects)
	gcc -o myprog $(objects) -lrt

assignment.o: assignment.c $(headers)
	gcc -c assignment.c

AccessControl.o: AccessControl.c
	gcc -c AccessControl.c

auditSite.o: auditSite.c
	gcc -c auditSite.c

update.o: update.c AccessControl.h
	gcc -c update.c

backup.o: backup.c AccessControl.h
	gcc -c backup.c

demandBackup:
	gcc demandBackup.c -o demandBackup

demandUpdate:
	gcc demandUpdate.c -o demandUpdate

clean:
	rm myprog demandBackup demandUpdate $(objects)
