#ifndef __autoexposure_h__
#define __autoexposure_h__

#include "../readconfig/read_config.h"

void init_coeff(app_cfg *appCfg);
void proseccingEG(cv::Mat &frame,int fd);
int  getexposure();
int  getgain();

#endif
