#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/wait.h>

int status;

void Audit()
{
  //open message queueto report back on success or failure
  mqd_t mq;
  mq = mq_open("/msgQueue",O_WRONLY);

  char createLog[100];
  // the command for creating the accesslog.txt which contains log information about the intranet folder
  strcpy(createLog,"ausearch -f /var/www/html/intranet/ > /var/www/accesslog.txt");

  //if audit fails
  if(system(createLog)<0){
    // error
    char err[500];
    strcpy(err,"audit error: ");
    strcat(err, strerror(errno));
    mq_send(mq,err,1024,0);
  }
  //if audit is a success
  else{
    // success
    char msg[500];
    strcpy(msg,"audit success");
    mq_send(mq,msg,1024,0);
  }
  //wait 5 secobds
  sleep(5);


}
