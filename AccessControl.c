#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>

mqd_t mq;

//locks specified location
void LockFolder(char * dir){
  //open message queue to report back on process
  mq = mq_open("/msgQueue",O_WRONLY);

 //pervents read,write,execute
  char mode[] = "0700";
  int i;
  i = strtol(mode, 0, 8);

  //attempt lock
  if (chmod (dir,i) < 0)
  {
     //error locking
     char err[500];
     strcpy(err,"error locking: ");
     strcat(err, strerror(errno));
     mq_send(mq,err,1024,0);
     exit(1);
  }
  else{
    //if lock is successful
    char msg[500];
    strcpy(msg,"locking success");
    mq_send(mq,msg,1024,0);
  }
}

//unlocks specified location
void UnlockFolder(char * dir){
  mq = mq_open("/msgQueue",O_WRONLY);
  //Everybody can read, write to, or execute
  char mode1[] = "0777";
  int i = strtol(mode1, 0, 8);
  //attempt unlock
  if (chmod (dir,i) < 0)
  {
     //error unlocking
     char err[500];
     strcpy(err,"error unlocking: ");
     strcat(err, strerror(errno));
     mq_send(mq,err,1024,0);
     exit(1);
  }
  else{
    //if unlock is successful
    char msg[500];
    strcpy(msg,"unlocking success");
    mq_send(mq,msg,1024,0);
  }
}
