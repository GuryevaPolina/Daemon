#include <syslog.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>

char pathToWorkingDirectory[255];
int period;


void sig_handler(int signo)
{
    if(signo == SIGTERM)
    {
        syslog(LOG_INFO, "SIGTERM has been caught! Exiting...");
        if(remove("run/daemon.pid") != 0)
        {
            syslog(LOG_ERR, "Failed to remove the pid file. Error number is %d", errno);
            exit(1);
        }
        exit(0);
    }
}

void handle_signals()
{
    if(signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        syslog(LOG_ERR, "Error! Can't catch SIGTERM");
        exit(1);
    }
}

void daemonise()
{
    pid_t pid, sid;
    FILE *pid_fp;
    
    syslog(LOG_INFO, "Starting daemonisation.");
    
    //First fork
    pid = fork();
    if(pid < 0)
    {
        syslog(LOG_ERR, "Error occured in the first fork while daemonising. Error number is %d", errno);
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
    if(sid < 0)
    {
        syslog(LOG_ERR, "Error occured in making a new session while daemonising. Error number is %d", errno);
        exit(1);
    }
    syslog(LOG_INFO, "New session was created successfuly!");
    
    //Second fork
    pid = fork();
    if(pid < 0)
    {
        syslog(LOG_ERR, "Error occured in the second fork while daemonising. Error number is %d", errno);
        exit(1);
    }
    
    if(pid > 0)
    {
        syslog(LOG_INFO, "Second fork successful (Parent)");
        exit(0);
    }
    syslog(LOG_INFO, "Second fork successful (Child)");
    
    pid = getpid();
    
    //Change working directory to Home directory
    if(chdir(getenv("HOME")) == -1)
    {
        syslog(LOG_ERR, "Failed to change working directory while daemonising. Error number is %d", errno);
        exit(1);
    }
    
    //Grant all permisions for all files and directories created by the daemon
    umask(0);
    
    //Redirect std IO
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    if(open("/dev/null",O_RDONLY) == -1)
    {
        syslog(LOG_ERR, "Failed to reopen stdin while daemonising. Error number is %d", errno);
        exit(1);
    }
    if(open("/dev/null",O_WRONLY) == -1)
    {
        syslog(LOG_ERR, "Failed to reopen stdout while daemonising. Error number is %d", errno);
        exit(1);
    }
    if(open("/dev/null",O_RDWR) == -1)
    {
        syslog(LOG_ERR, "Failed to reopen stderr while daemonising. Error number is %d", errno);
        exit(1);
    }
    
    //Create a pid file
    mkdir("run/", 0777);
    pid_fp = fopen("run/daemon.pid", "w");
    if(pid_fp == NULL)
    {
        syslog(LOG_ERR, "Failed to create a pid file while daemonising. Error number is %d", errno);
        exit(1);
    }
    if(fprintf(pid_fp, "%d\n", pid) < 0)
    {
        syslog(LOG_ERR, "Failed to write pid to pid file while daemonising. Error number is %d, trying to remove file", errno);
        fclose(pid_fp);
        if(remove("run/daemon.pid") != 0)
        {
            syslog(LOG_ERR, "Failed to remove pid file. Error number is %d", errno);
        }
        exit(1);
    }
    fclose(pid_fp);
}

void parseConfig(char* pathToConfigFile) {
    FILE* configFile = fopen(pathToConfigFile, "r");
    if (configFile == NULL) {
        syslog(LOG_ERR, "Config file is not found.", errno);
        // remove("run/daemon.pid");
        exit(1);
    }
    syslog(LOG_DEBUG, "Config file was opened.");
    char s[256];
    if (fgets(s, 255, configFile) != NULL) {
        sscanf(s, "%i", &period);
        syslog(LOG_DEBUG, "Period is 120 sec.");
    }
    fclose(configFile);
    syslog(LOG_DEBUG, "Config file was parsed.");
}

void removeFiles() {
    DIR *d;
    struct dirent *dir;
    d = opendir(pathToWorkingDirectory);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            FILE *file;
            struct stat st;
            char pathToFile[255];
            strcpy(pathToFile, pathToWorkingDirectory);
            strcat(pathToFile, dir->d_name);
            
            if((file = fopen(pathToFile, "rb")) == NULL) {
                syslog(LOG_DEBUG, "Cannot open file from directory.");
            }
            stat("test", &st);
            int res = 0;
            if (st.st_birthtime > 60000) {
                res = remove(pathToFile);
                if (res == 0) {
                    syslog(LOG_DEBUG, "File: %s was removed.", pathToFile);
                } else {
                    syslog(LOG_DEBUG, "Cannot delete file.");
                }
            }
            fclose(file);
        }
        closedir(d);
    }
}

int main(int argc, char* argv[])
{
    pid_t pid;
    FILE *pid_fp;
    
    char pathToConfigFile[255];
    char currentDirectory[255];
    getcwd(currentDirectory, 255);
    syslog(LOG_INFO, "path to currecnt directory: %s", currentDirectory);
    
    if (argc != 2)
    {
        syslog(LOG_ERR, "Must be two command line arguments.");
        exit(1);
    }
    
    strcpy(pathToConfigFile, currentDirectory);
    strcat(pathToConfigFile, "/config.txt");
    syslog(LOG_INFO, "Path to config was saved.");
    strcpy(pathToWorkingDirectory, currentDirectory);
    strcat(pathToWorkingDirectory, "/test/");
    syslog(LOG_INFO, "Path to working directory was saved.");
    
    chdir(getenv("HOME"));
    if (!strcmp(argv[1], "start"))
    {
        pid_fp = fopen("run/daemon.pid", "r");
        if(pid_fp == NULL)
        {
            syslog(LOG_INFO, "Starting daemon.");
            daemonise();
            handle_signals();
        }
        else
        {
            syslog(LOG_INFO, "Daemon already started.");
            fclose(pid_fp);
            exit(0);
        }
    }
    if (!strcmp(argv[1], "stop"))
    {
        pid_fp = fopen("run/daemon.pid", "r");
        if(pid_fp == NULL)
        {
            syslog(LOG_INFO, "Daemon isn't started.");
            exit(0);
        }
        else
        {
            syslog(LOG_INFO, "Stopping daemon");
            if (fscanf(pid_fp, "%d", &pid) < 0)
            {
                syslog(LOG_ERR, "Failed to read daemon pid. Error number is %d", errno);
                fclose(pid_fp);
                exit(1);
            }
            fclose(pid_fp);
            if (pid >= 0)
            {
                kill(pid, SIGTERM);
            }
            else
            {
                syslog(LOG_ERR, "Uncorrect pid.");
                exit(1);
            }
            exit(0);
        }
    }
    
    parseConfig(pathToConfigFile);
    
    while(1)
    {
        removeFiles();
        sleep(period * 1000);
    }
    return 0;
}
