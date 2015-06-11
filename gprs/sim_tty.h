#ifndef SIMTTY_H_
#define SIMTTY_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <cstring>

#define BAUDRATE B115200

//#define _POSIX_SOURCE 1

class Sim{
public:
  Sim();
  Sim(char *name);
  void initSim();
  void connectGPRS();
  void sendCmdSim(char *cmd,int n,int pause);
  int writeToSim(char *s,int n);
  void readFromSim(char *s,int *n,int r);
  void seticnon(int s);
  char getStatusConnect();
  void sendHTTPrequast(char *httpr);
  ~Sim(void);
private:
   void openSim(char *name);
   void getDataFromHTTPGET(void);
private:
 int fd,c, res;
 struct termios oldtio,newtio;
 char buf[1024];
 char buff[1024];
 char status_connect; 
};

#endif
