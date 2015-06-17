#include <iostream>
#include <ctype.h>
#include <unistd.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include "autoexposure.h"
#include "../cam-config/camconfig.h"

using namespace cv;

cv::Mat *frameCh;

void my_mean(cv::Mat& frame,int *mean){
    int i,j,ch;

    if (frameCh==NULL) frameCh=new cv::Mat[3];
    cv::split(frame,frameCh);

    long int summ=0;
    long int summ_block=0;

    for (ch=0;ch<3;ch++) {
        summ=0;
        for (i = 0; i < frameCh[ch].rows * frameCh[ch].cols / 300; i++) {
            summ_block = 0;
            for (j = 0; j < 300; j++) {
                summ_block += frameCh[ch].at < unsigned
                char > (300 * i + j);
            }
            summ += summ_block / 300;
        };
        summ=summ/1024;
        mean[ch]=summ;
    }
}


int E0=500;
int G0=200;
int Emin=50;
int Emax=2000;
int Gmin=100;
int Gmax=3000;
float ka=1.0;
float kg=1.0;
float aE=0.1;
float aG=0.1;
int E=E0;
int G=G0;
int Cth=127;
int Enc,Gnc;

void init_coeff(app_cfg *appCfg){
	E0=appCfg->E0;
	G0=appCfg->G0;
	Emax=appCfg->Emax;
	Emin=appCfg->Emin;
	Gmax=appCfg->Gmax;
	Gmin=appCfg->Gmin;
	ka=appCfg->ka;
	kg=appCfg->kg;
	aE=appCfg->aE;
	aG=appCfg->aG;
	Cth=appCfg->Cn;
	Gnc=G0;
	Enc=E0;
}

typedef struct _exposure_data{
	cv::Mat	frame;
	int fd;
	int E;
	int G;
} exposure_data;

void *correctEandG(void *data) {
    static int En_1=E0;
    static int Gn_1=G0;


    exposure_data *frame_data=(exposure_data*)data;

    Scalar cvMean=mean(frame_data->frame);
    int CmeanN=(int)(cvMean(0));//+cvMean(1)+cvMean(2))/3;

    if (CmeanN > Cth) {
        if (Gnc < Gmax && Gnc > Gmin) {
  	  Gn_1=G;
          //printf("1 G mean = %d !! %d + %f -->>",CmeanN, Gn_1 ,(aG)* kg * (Cth - CmeanN));
          Gnc =  Gn_1 + (1 - aG) * kg * (Cth - CmeanN);
	  //printf(" Gnc %d\n",Gnc);
          G = Gnc;
	} else {
 	  En_1=E;
          //printf("1 E mean = %d !! %d + %f -->>",CmeanN, En_1 ,(aE)* ka * (Cth - CmeanN));
          Enc =   En_1 + aE * ka * (Cth - CmeanN);
          //printf(" Enc %d\n",Enc);
          if (Enc < Emax && Enc > Emin) {
	   E = Enc;
	  }
	}
    } else {
        En_1=E;
        //printf("2 mean = %d !! %d + %f -->>",CmeanN, En_1 ,((aE) * ka * (Cth - CmeanN)));
        Enc = En_1 + aE * ka * (Cth - CmeanN);
        //printf(" Enc %d\n",Enc);
        if (Enc > Emin && Enc < Emax) {
	  E = Enc;
	} else {
          Gn_1=G;
          //printf("2 G mean = %d !! %d + %f -->>",CmeanN, Gn_1 ,(aG)* kg * (Cth - CmeanN));
	  Gnc =  Gn_1 + (1 - aG) * kg * (Cth - CmeanN);
	  //printf(" Gnc %d\n",Gnc);
	  if (Gnc > Gmin && Gnc < Gmax){
		    G = Gnc;
	  }  
        }	

    }    
    frame_data->E=E;
    frame_data->G=G;
    set_exposure(frame_data->E,frame_data->fd);
    set_gain(frame_data->G,frame_data->fd);
    //printf("E = %d  G = %d \n",E,G);
};



pthread_t thexp;
exposure_data gainexposure;

void proseccingEG(cv::Mat &frame,int fd){

	gainexposure.frame=frame;
	gainexposure.fd=fd;
	pthread_create(&thexp,NULL,correctEandG,&gainexposure);

}

