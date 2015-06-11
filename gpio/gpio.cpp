#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_gpio(int gpio) {
int fd;
char buf[256];

 fd = open("/sys/class/gpio/export", O_WRONLY);
  sprintf(buf, "%d", gpio); 
  write(fd, buf, strlen(buf));
 close(fd);
}

void set_direct_gpio(int gpio,char* in_out) {
int fd;
char buf[256]; 

 sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
  fd = open(buf, O_WRONLY);
  write(fd, in_out, strlen(in_out));
 close(fd);
}

void set_clr_gpio(int gpio,char flag) {
int fd;
char buf[256]; 

 sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
 fd = open(buf, O_WRONLY);
  if (flag) write(fd, "1", 1);  else write(fd, "0", 1); 
 close(fd);
}


int get_value_gpio(int gpio) {
int fd;
char buf[256]; 
char value;

sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
fd = open(buf, O_RDONLY);
 read(fd, &value, 1);
close(fd);
return value;
}


void resetSim(void){

//create_gpio(160);
create_gpio(161);
//create_gpio(162);
//create_gpio(163);
//create_gpio(164);

//set_direct_gpio(160,"out");
set_direct_gpio(161,"out");
//set_direct_gpio(162,"out");
//set_direct_gpio(163,"out");
//set_direct_gpio(164,"out");

set_clr_gpio(161,1);
sleep(1);
set_clr_gpio(161,0);
sleep(2);

//set_clr_gpio(162,0);
//set_clr_gpio(164,0);

}
