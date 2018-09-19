#include <syslog.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void daemonise()
{
  pid_t pid, sid;

  //First fork
  pid = fork();
  if(pid < 0)
  {
    syslog(LOG_ERR, "Error occured in the first fork while daemonising");
    exit(1);
  }

  if(pid > 0)
  {
    syslog(LOG_INFO, "First fork successful (Parent)");
    exit(0);
  }
  syslog(LOG_INFO, "First fork successful (Child)");

  //Create a new session
  sid = setsid();
  if (sid < 0) 
  {
    syslog(LOG_ERR, "Error occured in making a new session while daemonising");
    exit(1);
  }
  syslog(LOG_INFO, "New session was created successfuly!");

  //Second fork
  pid = fork();
  if(pid < 0)
  {
    syslog(LOG_ERR, "Error occured in the second fork while daemonising");
    exit(1);
  }

  if(pid > 0)
  {
    syslog(LOG_INFO, "Second fork successful (Parent)");
    exit(0);
  }
  syslog(LOG_INFO, "Second fork successful (Child)");

  //Change working directory to /
  if (chdir("/") == -1)
  {
    syslog(LOG_ERR, "Failed to change working directory while daemonising");
    exit(1);
  }
  //Grant all permisions for all files and directories created by the daemon
  umask(0);

  //Redirect std IO
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  if (open("/dev/null",O_RDONLY) == -1) 
  {
    syslog(LOG_ERR, "Failed to reopen stdin while daemonising");
    exit(1);
  }
  if (open("/dev/null",O_WRONLY) == -1) 
  {
    syslog(LOG_ERR, "failed to reopen stdout while daemonising");
    exit(1);
  }
  if (open("/dev/null",O_RDWR) == -1) 
  {
    syslog(LOG_ERR, "failed to reopen stderr while daemonising");
    exit(1);
  }

}

int main()
{
  daemonise();

  while(1)
  {
    sleep(1);
  }
  return 0;
}
