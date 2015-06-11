#ifndef GPIO_H_
#define GPIO_H_

void create_gpio(int gpio);
void set_direct_gpio(int gpio,char* in_out);
void set_clr_gpio(int gpio,char flag);
int get_value_gpio(int gpio);
void resetSim(void);

#endif
