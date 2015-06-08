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

void config_read(app_cfg *appCfg){

    FILE *f;
    char str_url[1024];

    f=fopen("cam.cfg","r");

    if (f){
        fscanf(f,"%s",str_url);
        int len=strlen(str_url);
        appCfg->url=0;
        appCfg->url=new char[len];
        strcpy(appCfg->url,str_url);
        fscanf(f,"%d",&appCfg->type_connect);
        fscanf(f,"%d",&appCfg->timeout_send_data);
        fscanf(f,"%d",&appCfg->id);
        fscanf(f,"%d",&appCfg->gpio);
        fscanf(f,"%d",&appCfg->Cn);
        fscanf(f,"%d",&appCfg->E0);
        fscanf(f,"%d",&appCfg->G0);

        fscanf(f,"%d",&appCfg->Emin);
        fscanf(f,"%d",&appCfg->Emax);
        fscanf(f,"%d",&appCfg->Gmin);
        fscanf(f,"%d",&appCfg->Gmax);


        fscanf(f,"%f",&appCfg->ka);
        fscanf(f,"%f",&appCfg->kg);
        fscanf(f,"%f",&appCfg->aE);
        fscanf(f,"%f",&appCfg->aG);
        fscanf(f,"%d",&appCfg->timeout_exposure);
    }
    fclose(f);
}
