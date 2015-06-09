	#include"read_config.h"
#include <stdio.h>
#include <string.h>

/*
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
 */

void read_number_int(FILE *f,int *n){
    int d;
    char inStr[1024];
    char *paserStr;

    fscanf(f, "%s", inStr);
    paserStr=strchr(inStr,'=');
    sscanf(paserStr+1, "%d", &d);
    printf("%d \n",d);
    *n=d;
}

void read_number_float(FILE *f,float *n){
    float d;
    char inStr[1024];
    char *paserStr;

    fscanf(f, "%s", inStr);
    paserStr=strchr(inStr,'=');
    sscanf(paserStr+1, "%f", &d);
    printf("%f \n",d);
    *n=d;
}

void config_read(app_cfg *appCfg){

    FILE *f;
    char inStr[1024];
    char str_url[1024];
    char *paserStr;

    f=fopen("cam.cfg","r");

    if (f){
        fscanf(f, "%s", inStr);
        paserStr=strchr(inStr,'=');
        sscanf(paserStr+1, "%s", str_url);
        appCfg->url = 0;
        int len = strlen(str_url);
        appCfg->url = new char[len];
        strcpy(appCfg->url, str_url);
        printf("%s\n",appCfg->url);

        read_number_int(f,&appCfg->type_connect);
        read_number_int(f,&appCfg->timeout_send_data);

        read_number_int(f,&appCfg->id);
        read_number_int(f,&appCfg->gpio);
        read_number_int(f,&appCfg->Cn);
        read_number_int(f,&appCfg->E0);
        read_number_int(f,&appCfg->G0);

        read_number_int(f,&appCfg->Emin);
        read_number_int(f,&appCfg->Emax);
        read_number_int(f,&appCfg->Gmin);
        read_number_int(f,&appCfg->Gmax);

        read_number_float(f,&appCfg->ka);
        read_number_float(f,&appCfg->kg);
        read_number_float(f,&appCfg->aE);
        read_number_float(f,&appCfg->aG);
        read_number_int(f,&appCfg->timeout_exposure);
    }
    fclose(f);
}
