#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
  int fd;
  char * fFile = "/tmp/fifoFile";

  mkfifo(fFile,0666);

  fd = open(fFile,O_WRONLY);

  write(fd,"update",sizeof("update"));

  close(fd);
  unlink(fFile);
}
