#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>

#include "AccessControl.h"

//will hold status of child process
int status;

int BackupLive()
{
  //open message queue to send success/failure of backup
  mqd_t mq;
  mq = mq_open("/msgQueue",O_WRONLY);

  char *buf = "/var/www/html/intranet";
  //lock intranet folder to prevent changes during backup
  LockFolder(buf);

  pid_t pid =fork();

  //child process will perfrom backup
  if(pid==0){
    sleep(5);

    time_t now;
    time(&now);
    //create backup folder
    char dir[500];
    strcpy(dir,"/var/www/backup/");
    //get current date/time
    struct tm * datetime;
    datetime = localtime ( &now );
    strcat(dir, asctime (datetime));
    mkdir(dir, 0777);

    //copy files and folder to backup folder
    char* copy_cmd[] = {"cp","-R","/var/www/html/live" , dir, NULL};
    execvp ("/bin/cp", copy_cmd);
    //if execvp fails
    exit(EXIT_FAILURE);
  }
  else if(pid>0){
    //wait for child process to finish
    pid = wait(&status);

    //if execvp was executed
    if(status==0){
      mq_send(mq,"backup success",1024,0);
    }
    else{
      mq_send(mq,"backup error",1024,0);
    }

    //unlock intranet folder
    UnlockFolder(buf);

  }
  else{
    mq_send(mq,"fork error in backup",1024,0);
  }
  //return status of backup
  return status;
}
