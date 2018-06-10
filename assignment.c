#include<stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>

#include "backup.h"
#include "update.h"
#include "auditSite.h"
#include "AccessControl.h"

void StartBackup(){
  BackupLive();
}

void StartUpdate(){
  int stat = BackupLive();
  if(stat==0){
    UpdateLive();
  }
  else{
    mqd_t mq;
    mq = mq_open("/msgQueue",O_WRONLY);
    // error
    char err[500];
    strcpy(err,"update error: backup failed ");
    mq_send(mq,err,1024,0);
  }
}

void Setup(){
  //set the watch on the intranet folder
  char setWatch[100];
  strcpy(setWatch, "auditctl -w /var/www/html/intranet -p rwxa");

  if (system(setWatch) < 0){
    //if watch failed
    mqd_t mq;
    mq = mq_open("/msgQueue",O_WRONLY);
    // error
    char err[500];
    strcpy(err,"directory watch error: ");
    strcat(err, strerror(errno));
    mq_send(mq,err,1024,0);
  }

  //lock live folder
  char *buf = {"/var/www/html/live"};
  LockFolder(buf);
}

int main()
{
  time_t now;
  struct tm nightTime;
  double seconds;
  time(&now);  /* get current time; same as: now = time(NULL)  */
  nightTime = *localtime(&now);
  nightTime.tm_hour = 21;
  nightTime.tm_min = 10;
  nightTime.tm_sec = 0;


  int ppid;

  Setup();

  int pid = fork();

  if(pid>0){
    //create orphan
    exit(EXIT_SUCCESS);

  }else if (pid==0){

    //create daemon
    //elevate orphan to session leader
    if (setsid() < 0) { exit(EXIT_FAILURE); }

    //set file mode creation mask to 0
    umask(0);

    //change working dir to root
    if (chdir("/") < 0 ) { exit(EXIT_FAILURE); }

    //close all file descriptors
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
       close (x);
    }

    int fd;
    char * fFile = "/tmp/fifoFile";

    pid_t pid2 = fork();

    if(pid2>0)
    {
      mqd_t mq;
      struct mq_attr queue_attributes;
      char buffer[1024+1];

      //set message queue attributes
      queue_attributes.mq_flags=0;
      queue_attributes.mq_maxmsg=10;
      queue_attributes.mq_msgsize=1024;
      queue_attributes.mq_curmsgs=0;

      //create/open message queue
      mq = mq_open("/msgQueue",O_CREAT | O_RDONLY, 0644, &queue_attributes);

      while(1){
        ssize_t bytes_read;

        //read in message from message queue
        bytes_read = mq_receive(mq,buffer,1024,NULL);
        //terminate string
        buffer[bytes_read]='\0';

        char logM[200];
        strcpy(logM,buffer);

        //if an error
        if(strstr(buffer,"error")!=NULL){
         //output message to sysylog
         openlog("WebsiteDaemon2",LOG_PID|LOG_CONS|LOG_PERROR,LOG_SYSLOG);
         syslog(LOG_ERR,logM);
         closelog();
        }
        //if a success
        else if(strstr(buffer,"success")!=NULL){
         //output message to sysylog
         openlog("WebsiteDaemon2",LOG_PID|LOG_CONS,LOG_SYSLOG);
         syslog(LOG_INFO,logM);
         closelog();
        }

      }

      mq_close(mq);
      mq_unlink("msgQueue");
    }
    else if(pid2==0){

      while(1){

        Audit();

        char buf[10]="";

        //read in from fifofile
        fd = open(fFile,O_RDONLY);
        read(fd,buf,10);


        time(&now);
        seconds = difftime(now,mktime(&nightTime));
        //if pre selected time has been reached
        if (seconds < 0 && seconds >-5) {
          StartUpdate();
        }

        //if on demand backup
        else if(strcmp(buf,"backup")==0){
          StartBackup();
        }

        //if on demand update
        else if(strcmp(buf,"update")==0){
          StartUpdate();
        }

        //close fd for fifo
        close(fd);
      }
    }
    //if forking in daemon fails
    else{
      mqd_t mq;
      mq = mq_open("/msgQueue",O_WRONLY);

      mq_send(mq,"fork error in daemon",1024,0);
    }

  }
  //if forking to create daemon fails
  else{
    mqd_t mq;
    mq = mq_open("/msgQueue",O_WRONLY);

    mq_send(mq,"fork error in main",1024,0);
  }


}
