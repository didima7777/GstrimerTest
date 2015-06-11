#include "sim_tty.h"

Sim::Sim(void) {
  openSim((char*)"/dev/ttymxc2");
}

Sim::Sim(char *name){
  openSim(name);
}

void Sim::openSim(char *name){
        /* 
          Open modem device for reading and writing and not as controlling tty
          because we don't want to get killed if linenoise sends CTRL-C.
        */
         fd = open(name, O_RDWR | O_NOCTTY ); 
         if (fd <0) {
		perror(name); 
	} else {

         tcgetattr(fd,&oldtio);
         bzero(&newtio, sizeof(newtio)); 
        
        /* 
          BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
          CRTSCTS : output hardware flow control (only used if the cable has
                    all necessary lines. See sect. 7 of Serial-HOWTO)
          CS8     : 8n1 (8bit,no parity,1 stopbit)
          CLOCAL  : local connection, no modem contol
          CREAD   : enable receiving characters
        */
         newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
         
        /*
          IGNPAR  : ignore bytes with parity errors
          ICRNL   : map CR to NL (NL- end string /0)
          otherwise make device raw (no other input processing)
        */
         newtio.c_iflag = IGNPAR;//| ICRNL;
         
        /*
         Raw output.
        */
         newtio.c_oflag = 0;
         
        /*
          ICANON  : enable canonical input
          disable all echo functionality, and don't send signals to calling program
        */
         newtio.c_lflag = ICANON;
         
        /* 
          initialize all control characters 
          default values can be found in /usr/include/termios.h, and are given
          in the comments, but we don't need them here
        */
         newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
         newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
         newtio.c_cc[VERASE]   = 0;     /* del */
         newtio.c_cc[VKILL]    = 0;     /* @ */
         newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
         newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
         newtio.c_cc[VMIN]     = 3;     /* blocking read until 1 character arrives */
         newtio.c_cc[VSWTC]    = 0;     /* '\0' */
         newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
         newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
         newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
         newtio.c_cc[VEOL]     = 0;     /* '\0' */
         newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
         newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
         newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
         newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
         newtio.c_cc[VEOL2]    = 0;     /* '\0' */
        
        /* 
          now clean the modem line and activate the settings for the port
        */
         //tcflush(fd, TCIFLUSH);

         tcsetattr(fd,TCSANOW,&newtio);

	 status_connect=0;
	}
}

Sim::~Sim(void){
        /* restore the old port settings */
 if (fd >0) {
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
 }	
}

void Sim::initSim(){
  sendCmdSim("AT\r",20,1);
  sendCmdSim("ATE0\r",20,1);
}

void Sim::connectGPRS(){
 FILE *file;
 char  strcfg[128];
 file=fopen("gprs.cfg","r");
 if (file>0) {
   while(!feof(file)){
    bzero(strcfg, 128); 
    fscanf(file,"%s",strcfg);
    if (memcmp(strcfg,"end",3)==0) break;
    strcfg[strlen(strcfg)]='\r';
    sendCmdSim(strcfg,20,4);
    if (memcmp(buff,"OK",2)==0) {
        status_connect=1;
    } else {
	status_connect=0;
	return;
    }
  }
  if (status_connect) printf("Coonect to GPRS OK!!!\n");
  fclose(file); 
 } else {
  sendCmdSim("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r",20,2);
      if (memcmp(buff,"OK",2)==0) {
        status_connect=1;
    }
  sendCmdSim("AT+SAPBR=3,1,\"APN\",\"internet\"\r",20,2);
      if (memcmp(buff,"OK",2)==0) {
        status_connect=1;
    }
  sendCmdSim("AT+SAPBR=1,1\r",20,5);
      if (memcmp(buff,"OK",2)==0) {
        status_connect=1;
 	printf("Coonect to GPRS OK!!!\n");
    }
 }
}

char Sim::getStatusConnect(){ 
 return status_connect;
}

void Sim::sendCmdSim(char *cmd,int n,int pause){
 int r;
 printf("%d write %s\n",writeToSim(cmd,strlen(cmd)),cmd);
 sleep(pause);
 readFromSim(buff,&r,n);
// printf("1 read  %d: %s \n",r,buff);
 readFromSim(buff,&r,n);
// printf("2 read  %d: %s \n",r,buff);
}

void Sim::getDataFromHTTPGET(void){
  int n=20;
  int d;

  writeToSim("AT+HTTPREAD\r",strlen("AT+HTTPREAD\r"));
  sleep(2);
  readFromSim(buff,&n,20);
//  printf("1 read  %d: %s \n",n,buff);
  readFromSim(buff,&n,20);
//  printf("2 read  %d: %s \n",n,buff);

  char *paserStr=strchr(buff,':');
  sscanf(paserStr+1, "%d", &d);
  seticnon(0);
  readFromSim(buff,&n,d+6);
  printf("DATA read  %d: %s \n",n,buff);
  seticnon(1);

};

void Sim::sendHTTPrequast(char *httpr){

char requast[1024];
int  n;

bzero(requast, 1024);

 if (getStatusConnect()) {
	 sendCmdSim("AT+HTTPINIT\r",20,1);
	 sendCmdSim("AT+HTTPPARA=\"CID\",1\r",20,1);
	 sprintf(requast,"AT+HTTPPARA=\"URL\",\"%s\"\r",httpr);
 	 sendCmdSim(requast,20,1);
	 sendCmdSim("AT+HTTPACTION=0\r",20,5);
	 n=20;
	 readFromSim(buff,&n,30);
//	 printf("read  %d: %s \n",n,buff);
	 readFromSim(buff,&n,30);
//	 printf("read  %d: %s \n",n,buff);

	 getDataFromHTTPGET();

	 sendCmdSim("AT+HTTPTERM\r",20,2);

 }
}

void Sim::seticnon(int s){
	if (s) {
	 newtio.c_lflag = ICANON;
	 tcsetattr(fd,TCSANOW,&newtio);
	} else {
	 newtio.c_lflag = 0;
	 tcsetattr(fd,TCSANOW,&newtio);
	}
}

void Sim::readFromSim(char *s,int *n,int r) {
	bzero(buf, 1024);	 
	if (fd >0) {
	 *n=read(fd,buf,r); 
  	 strcpy(s,buf);
	}
};

int Sim::writeToSim(char *s,int n) {
    int r=-1;	
    if (fd>0) r=write(fd,s,n);
    return r; 	
};
