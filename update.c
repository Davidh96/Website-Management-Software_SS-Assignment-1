#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>

#include "AccessControl.h"
//store status of child
int status;

void UpdateLive()
{
  //open message queue to send success/failure of backup
  mqd_t mq;
  mq = mq_open("/msgQueue",O_WRONLY);

  //lock intranet folder
  char *buf = {"/var/www/html/intranet"};
  LockFolder(buf);

  pid_t pid = fork();

  //child process will carry out update
  if(pid==0){
    sleep(5);
    //copy only files and folder to live folder that are not already present
    char* copy_cmd[] = {"cp","-u","-R","/var/www/html/intranet/.", "/var/www/html/live", NULL};
    execvp("/bin/cp", copy_cmd);
    //if execvp fails
    exit(EXIT_FAILURE);
  }
  else if(pid>0){
    //wait for child process to finish
    pid = wait(&status);

    if(status==0){
      mq_send(mq,"update success",1024,0);

      //add update to update list
      FILE *fptr;
      fptr = fopen("/var/www/updateList.txt","a+");
      //if fopen fails
      if(fptr==NULL){
        mqd_t mq;
        mq = mq_open("/msgQueue",O_WRONLY);
        // error
        char err[500];
        strcpy(err,"list update error: ");
        strcat(err, strerror(errno));
        mq_send(mq,err,1024,0);
        exit(1);
      }

      time_t now;
      time(&now);
      //create backup folder
      char note[500];
      strcpy(note,"Update Completed: ");
      //get current dat/time
      struct tm * timeinfo;
      timeinfo = localtime ( &now );
      strcat(note, asctime (timeinfo));
      //give date/time of update
      fprintf(fptr,"%s",note);
      fclose(fptr);
    }
    else{
      mq_send(mq,"update error",1024,0);
    }

    //unlock intranet folder
    UnlockFolder(buf);


  }
  else{
    mq_send(mq,"fork error in update",1024,0);
  }

}
