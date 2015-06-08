//
// Created by dima on 04.06.15.
//

#ifndef TEST_READ_CONFIG_READ_CONFIG_H
#define TEST_READ_CONFIG_READ_CONFIG_H

typedef  struct _app_cfg{
    char *url;
    int type_connect; // 0 - Ethernet; 1 - GSM
    int timeout_send_data; // period send result of counting (sec)
    int id; // id device
    int gpio; // 0 - GPIO off,  1 - GPIO on
    int Cn;
    int E0;
    int G0;
    int Emin;
    int Emax;
    int Gmin;
    int Gmax;
    float ka;
    float kg;
    float aE;
    float aG;
    int timeout_exposure;
} app_cfg;

void config_read(app_cfg *appCfg);

#endif //TEST_READ_CONFIG_READ_CONFIG_H
