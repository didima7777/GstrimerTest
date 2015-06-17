#include <fstream>
#include <linux/videodev2.h>
#include <linux/mxc_v4l2.h>
#include <sys/ioctl.h>
	
void set_exposure(int exp,int fd_video) {
	v4l2_control par_exp;
	par_exp.id=V4L2_CID_EXPOSURE;
	par_exp.value=exp;
	if (ioctl(fd_video, VIDIOC_S_CTRL, &par_exp) < 0)
	{
		printf("\nVIDIOC_S_CTRL failed\n");
	} else {
	// printf("shutter %d \n",par_exp.value);
	}
}


void set_gain(int gain,int fd_video) {
	v4l2_control par_exp;
	par_exp.id=V4L2_CID_GAIN;
	par_exp.value=gain;
	if (ioctl(fd_video, VIDIOC_S_CTRL, &par_exp) < 0)
	{
		printf("\nVIDIOC_S_CTRL failed\n");
	} else {
	 //printf("gain %d \n",par_exp.value);
	}
}
