#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>  
#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include <linux/mxc_v4l2.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "Counting/Counter.h"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "gst-server-lib/rtsp-server.h"
#include "cam-config/camconfig.h"
#include "autoexposure/autoexposure.h"
#include "readconfig/read_config.h"
#include "readconfig/read_config.h"
#include "http_requast/http_requast.h"
#include "bmp.h"

int default_exposure=500;
int fd_video;
GMainLoop *loop;
app_cfg AppCfg;


typedef struct _CustomData {
    gboolean is_live;
    GstElement *pipeline;
    GMainLoop *loop;
} CustomData;


    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    GstElement *pipeline_v1, *pipeline_v2;
    GstElement *source, *sink_app_1,*sink_app_2, *sink_file, *csp, *enc, *videotee, *parser,*gdppay;
    GstElement *queue1, *queue2;
    GstElement *rtp, *udpsink, *fakesink,*tcpserver;


void print_buffer (GstBuffer *buffer, const char *title);

static void convert(unsigned char *src, char *dst, int size) {
    for (int i = 0, j = 0; i < size * 3; i += 6, j += 2) //+2
    {
        dst[i] = src[j + 0];
        dst[i + 1] = src[j + 0];
        dst[i + 2] = src[j + 0];
        dst[i + 3] = src[j + 1];
        dst[i + 4] = src[j + 1];
        dst[i + 5] = src[j + 1];
    }
};

static GstFlowReturn new_buffer_list (GstAppSink *sink, gpointer user_data)
{ 
    GstBufferList *list = gst_app_sink_pull_buffer_list (sink);
    GstBufferListIterator *it = gst_buffer_list_iterate (list);
    GstBuffer *buffer;
    while (gst_buffer_list_iterator_next_group (it))
        while ((buffer = gst_buffer_list_iterator_next (it)) != NULL)
            print_buffer(buffer, "new_buffer_list");
    gst_buffer_list_iterator_free (it);
 
    return GST_FLOW_OK;
}

static void new_oes(GstAppSink *sink, gpointer user_data) {
    printf("###### eos #######\n");
}

static GstFlowReturn new_preroll(GstAppSink *sink, gpointer user_data) {
	printf("#####  new_preroll #######!!!\n");
	fd_video=0;
        g_object_get (G_OBJECT (source), "file-id", &fd_video, NULL);
/*
	printf("fd=%d!!!!\n",fd_video);
	struct v4l2_dbg_chip_ident chip;			    
	if (ioctl(fd_video, VIDIOC_DBG_G_CHIP_IDENT, &chip))
	{
		printf("\nVIDIOC_DBG_G_CHIP_IDENT failed.\n");
	} else {
	printf("\nTV decoder chip is %s !!!!\n", chip.match.name);
	}
*/
	set_exposure(default_exposure,fd_video);
        GstBuffer *buffer =  gst_app_sink_pull_preroll (sink);
        if (buffer) {
          print_buffer(buffer, "preroll");
   	  gst_buffer_unref(buffer);
        }
   return GST_FLOW_OK;
}

GMutex mutex;
Counter *cnter;

void init_counting(std::string& pathToConfig){
	cv::Size frameSize(640,480);	
	std::ifstream confZone(pathToConfig.c_str());

	cv::Mat detectionZone(4, 1, CV_32FC2);
	int x, y;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt1(x, y);
	detectionZone.at<cv::Point2f>(0) = pt1;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt2(x, y);
	detectionZone.at<cv::Point2f>(1) = pt2;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt3(x, y);
	detectionZone.at<cv::Point2f>(2) = pt3;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt4(x, y);
	detectionZone.at<cv::Point2f>(3) = pt4;

	int stripeNum = 3;
	double stripeThr = 0.1;

	cnter=new Counter(detectionZone, frameSize, stripeNum, stripeThr,0);
}

char name_file[30];
unsigned long int currentCount=0;
unsigned long int old_currentCount=0;
GstBuffer* buffer;
GstBuffer* buffer_rtp;
unsigned char flag_rtp=0;

IplImage *frame = NULL;
IplImage *m_RGB = NULL;
IplImage *m_Y = NULL;
unsigned char *buf_tmp = NULL;

char name[30];
int cnt = 0;
int cnt_http = 0;

void test_counting(){
	printf("Try open file: video_test1.avi\n");

	sprintf(name,"bmp/cap%d.bmp",cnt);
	cv::VideoCapture vidCapture(name);

	cv::Mat frame;
	printf("get first cap\n"); 
	vidCapture >> frame;
	cv::Size frameSize = frame.size();	

	std::ifstream confZone("conf.txt");

	printf("Read config \n"); 

	cv::Mat detectionZone(4, 1, CV_32FC2);
	int x, y;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt1(x, y);
	detectionZone.at<cv::Point2f>(0) = pt1;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt2(x, y);
	detectionZone.at<cv::Point2f>(1) = pt2;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt3(x, y);
	detectionZone.at<cv::Point2f>(2) = pt3;
	confZone >> x;
	confZone >> y;
	cv::Point2f pt4(x, y);
	detectionZone.at<cv::Point2f>(3) = pt4;

	int stripeNum = 1;
	double stripeThr = 0.1;

	Counter cnter(detectionZone, frameSize, stripeNum, stripeThr,0);
	printf("Start processing \n");
	while (cnt < 187)
	{		
		int currentCount = cnter.processFrame(frame);
		std::cout << cnt << " " << currentCount << std::endl;
		cnt++;
		sprintf(name,"bmp/cap%d.bmp",cnt);
		cv::VideoCapture vidCapture(name);
		vidCapture >> frame;
	}	

} 

char str_http_send[256];

static GstFlowReturn new_buffer(GstAppSink *sink, gpointer user_data) {
    int height =480; //480;//1944;
    int width = 640;//640;//2592;
    static int  cnt=0;
    static int expo=1;
	
    g_mutex_lock(&mutex);
    buffer =  gst_app_sink_pull_buffer (sink);
    if (flag_rtp==1) buffer_rtp = buffer;//gst_buffer_copy (buffer);

    if (buffer) {
            unsigned char *pData=(unsigned char*)GST_BUFFER_DATA(buffer);
            if (m_RGB==NULL) m_RGB = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	    if (m_Y==NULL) m_Y = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
            if (buf_tmp ==NULL) buf_tmp=new unsigned char[width*height*3];
            memcpy((void*)buf_tmp,(void*)pData,width*height*1.5);

//            convert(buf_tmp, m_RGB->imageData, height * width);
	    V4LWrapper_CvtColor((char*)buf_tmp, (char*)m_RGB->imageData,width,height);

	    cv::Mat frame=cv::Mat(480,640,CV_8UC3,m_RGB->imageData);

     	    if (cnt++>30*AppCfg.timeout_exposure) {
                //run thread send http GET  
   	        cv::Mat frame_exposure=cv::Mat(480,640,CV_8UC1,(char*)buf_tmp);
		proseccingEG(frame_exposure,fd_video);
   	 	cnt=0;
	    }
	    if (cnt_http++>30*AppCfg.timeout_send_data){
		sprintf(str_http_send,"count=%d",currentCount);
		printf("%s\n",str_http_send);
		send_http_requast(AppCfg.url,str_http_send);
		cnt_http=0;
   	    }

    	    currentCount = cnter->processFrame(frame);
	    if (old_currentCount!=currentCount) {
		printf("counting = %ld\n",currentCount);
		old_currentCount=currentCount;
	 	    sprintf(name_file,"cap_cnt_%d.bmp",currentCount);
	 	    cv::Mat mat_img(m_RGB);
		    cv::imwrite(name_file, mat_img);
		    mat_img.release();
		    cvReleaseImage(&m_RGB);
	    }
           gst_buffer_unref(buffer);
    }
    g_mutex_unlock(&mutex);
    return GST_FLOW_OK;
}

void print_buffer (GstBuffer *buffer, const char *title) {
    g_mutex_lock(&mutex);
    GstCaps *caps = gst_buffer_get_caps(buffer);
    for (uint j = 0; j < gst_caps_get_size(caps); ++j) {
        GstStructure *structure = gst_caps_get_structure(caps, j);
        printf("%s{%ss}: ", (title), (gst_structure_get_name(structure)));
        for (int i = 0; i < gst_structure_n_fields(structure); ++i) {
            if (i != 0)
                printf(", ");
            const char *name = gst_structure_nth_field_name(structure, i);
            GType type = gst_structure_get_field_type(structure, name);
            printf("%s[%s]", (name), (g_type_name(type)));
        }
        printf("\n");
    }
    g_mutex_unlock(&mutex);
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer user_data) {
    int i;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old_state, new_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
            printf("[%s]: %s -> %s\n", GST_OBJECT_NAME(msg->src), gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
            if (strcmp(msg->src->name,"source")==0) {
                if (strcmp(gst_element_state_get_name(new_state),"READY")==0) {
                    g_object_get (G_OBJECT (source), "file-id", &fd_video, NULL);
/*
		    struct v4l2_dbg_chip_ident chip;
	   	    if (ioctl(fd_video, VIDIOC_DBG_G_CHIP_IDENT, &chip))
  			printf("\nVIDIOC_DBG_G_CHIP_IDENT failed.\n");
		   else 
 		    printf("\nTV decoder chip is %s !!!!\n", chip.match.name);		
*/
              }
            }
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *err;
            gst_message_parse_error(msg, &err, &debug);
            printf("[%s]: %s %s\n", GST_OBJECT_NAME(msg->src), err->message, debug);
            g_free(debug);
            g_error_free(err);
            break;
        }
        case GST_MESSAGE_EOS:
            /* end-of-stream */
            g_main_loop_quit(loop);
            break;
        default:
        {
/*
            printf("default \n");
            const GstStructure *structure = gst_message_get_structure(msg);
            if (structure) {
                printf("%s{%s} ", gst_message_type_get_name(msg->type), gst_structure_get_name(structure));
                for (i = 0; i < gst_structure_n_fields(structure); ++i) {
                    if (i != 0) printf(", ");
                    const char *name = gst_structure_nth_field_name(structure, i);
                    GType type = gst_structure_get_field_type(structure, name);
                    printf("%s", name);
                    printf("[%s]", g_type_name(type));
                }
            printf("\n");
            } else {
                //printf("info: %i %s type: %i\n", (int) (msg->timestamp), GST_MESSAGE_TYPE_NAME(msg), msg->type);
                // printf("%s{}\n", gst_message_type_get_name (msg->type));
            }
*/
            break;
        }
    }
    return true;
}

void add_cliden (GstElement* object, gchararray arg0, gint arg1, gpointer user_data){
    printf("add clien \n");
}


typedef struct _App App;
struct _App
{
    GstElement *videosink;
};
App s_app;

typedef struct {
    App *glblapp;
    GstClockTime timestamp;
} Context;

static void
need_data (GstElement *appsrc, guint unused, Context *ctx)
{
    GstFlowReturn ret;
//  GstBuffer* buffer;
//  buffer =  gst_app_sink_pull_buffer (GST_APP_SINK(ctx->glblapp->videosink));

    if (buffer_rtp) {
        GST_BUFFER_TIMESTAMP(buffer_rtp) = ctx->timestamp;
        GST_BUFFER_DURATION (buffer_rtp) = gst_util_uint64_scale_int (1,GST_SECOND, 30);
        ctx->timestamp += GST_BUFFER_DURATION (buffer_rtp);
        g_signal_emit_by_name(appsrc, "push-buffer", buffer_rtp, &ret);
        //gst_object_unref (buffer_rtp);
    }

}

static void media_configure (GstRTSPMediaFactory *factory, GstRTSPMedia *media, App *app)
{    
    Context *ctx;
    GstElement *pipeline;
    GstElement *appsrc;
    pipeline = gst_rtsp_media_get_element(media);
    appsrc = gst_bin_get_by_name_recurse_up (GST_BIN (pipeline), "mysrc");
    gst_rtsp_media_set_reusable(media, TRUE);
    gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");
    g_object_set(G_OBJECT(appsrc), "max-bytes", gst_app_src_get_max_bytes(GST_APP_SRC(appsrc)), NULL);
    ctx = g_new0 (Context, 1);
    ctx->glblapp = app;
    ctx->timestamp = 0;
    g_signal_connect (appsrc, "need-data", (GCallback) need_data, ctx);
    printf("\nmedia_configure OK\n");
    flag_rtp=1;
    gst_object_unref (appsrc);
    gst_object_unref (pipeline);
}

void almost_c99_signal_handler(int signum)
{
 switch(signum)
  {
    case SIGABRT:
      fputs("Caught SIGABRT: usually caused by an abort() or assert()\n", stderr);
      break;
    case SIGFPE:
      fputs("Caught SIGFPE: arithmetic exception, such as divide by zero\n",
            stderr);
      break;
    case SIGILL:
      fputs("Caught SIGILL: illegal instruction\n", stderr);
      break;
    case SIGINT:{
      fputs("Caught SIGINT: interactive attention signal, probably a ctrl+c!!!!\n",
            stderr);
      g_main_loop_quit(loop);
      GMainLoop	*tmp_loop=get_rtsp_loop();
      if (tmp_loop!=0)	      g_main_loop_quit(tmp_loop);
      }
      return ;
      break;
    case SIGSEGV:
      fputs("Caught SIGSEGV: segfault\n", stderr);
      break;
    case SIGTERM:
    default:
      fputs("Caught SIGTERM: a termination request was sent to the program\n",
            stderr);
      _Exit(1);
      break;
  }
  _Exit(1);
}




void process_command(int); /* function prototype */

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int sockfd, newsockfd, portno, pid;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

void* server(void *t)
{
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 23589;//atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             error("ERROR on accept");
	 else {
             process_command(newsockfd);
	     close(newsockfd);
	}
     } /* end of while */
     pthread_exit((void*) t);
}

void process_command(int sock)
{
   int n;
   char buffer[256];
   int value;
   char cmd;
	   
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");
   printf("Here is the message: %s\n",buffer);
   sscanf(buffer,"%c %d",&cmd,&value);
   switch(cmd){
   case 's':
	set_exposure(value,fd_video);
    break;	
   case 'g': 
	set_gain(value,fd_video);
    break;	
   }
   bzero(buffer,256);
   sprintf(buffer,"I got your message %d \n",currentCount);
   n = write(sock,buffer,strlen(buffer));
   if (n < 0) error("ERROR writing to socket");
}


int main(int argc, char **argv) {


    /* Initialize GStreamer */
    signal(SIGABRT, almost_c99_signal_handler);
    signal(SIGFPE,  almost_c99_signal_handler);
    signal(SIGILL,  almost_c99_signal_handler);
    signal(SIGINT,  almost_c99_signal_handler);
    signal(SIGSEGV, almost_c99_signal_handler);
    signal(SIGTERM, almost_c99_signal_handler);
    

   pthread_t thread;
   pthread_attr_t attr;
   int rc;
   long t;
   void *status;
   App *app = &s_app;

   int debug=0;

   if (argc>1) {
   sscanf(argv[1],"%d",&debug);
	   if (debug) {
		   test_counting();
		   return 0; 
	    }
    }
   /* Initialize and set thread detached attribute */

   config_read(&AppCfg);
   init_coeff(&AppCfg);
	
   t=0;
   printf("Start gstreamer 0.1 and Counting\n");	
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   rc = pthread_create(&thread, &attr, server, (void *)t); 
   
    gst_init(&argc, &argv);
    std::string pathToConfig = "confZone1.txt";
    init_counting(pathToConfig);


    source = gst_element_factory_make("imxv4l2src", "source cam");
    queue1 = gst_element_factory_make("queue", "queue1");
    queue2 = gst_element_factory_make("queue", "queue2");
//  g_object_set( G_OBJECT(queue1), "max-size-buffers", 10, NULL);
        
     enc = gst_element_factory_make("vpuenc", "imxvpu");
     g_object_set(G_OBJECT(enc), "codec", 6, NULL);

    parser = gst_element_factory_make("h264parse", "h264parse");
   
    rtp = gst_element_factory_make("rtph264pay", "rtp");
    g_object_set( G_OBJECT(rtp), "config-interval", 1, NULL);	
    g_object_set( G_OBJECT(rtp), "pt", 96, NULL);
   

    fakesink = gst_element_factory_make("fakesink", "fakesink");
    udpsink = gst_element_factory_make("udpsink", "multiudpsink");
    g_object_set(G_OBJECT(udpsink), "host", "10.0.0.2", NULL);
    g_object_set(G_OBJECT(udpsink), "port", 5000, NULL);
    g_object_set(G_OBJECT(udpsink), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(udpsink), "async", FALSE, NULL);
    
    tcpserver = gst_element_factory_make("tcpserversink", "tcpserversink");
    g_object_set(G_OBJECT(tcpserver), "host", "10.0.0.1", NULL);
    g_object_set(G_OBJECT(tcpserver), "port", 8081, NULL);

    sink_app_1 = gst_element_factory_make("appsink", "appsink 1");
    sink_app_2 = gst_element_factory_make("appsink", "appsink 2");
    app->videosink = sink_app_2;
    gst_app_sink_set_drop(GST_APP_SINK (app->videosink), TRUE);
    gst_app_sink_set_max_buffers(GST_APP_SINK (app->videosink), 1);

    sink_file = gst_element_factory_make("filesink", "filesink");
    g_object_set(G_OBJECT(sink_file), "location", "t1.jpeg", NULL);
    
    videotee = gst_element_factory_make("tee", "videotee");

    g_object_set(G_OBJECT(source), "device", "/dev/video0", NULL);
//  g_object_set( G_OBJECT(source), "capture-mode", 0, NULL); 
    g_object_set( G_OBJECT(source), "fps-n", "30", NULL); 

    pipeline_v1 = gst_pipeline_new("cam-pipeline");
    pipeline_v2 = gst_pipeline_new("cam-pipeline");


    if (!pipeline_v1 || !pipeline_v2 || !source || !sink_app_1 || !sink_app_2 || !sink_file || !queue1 || !queue2 || !videotee || !enc || !parser || !tcpserver || !udpsink) {
        g_printerr("Not pipeline element could be created.\n");
        return -1;
    }
    if (!rtp) {
        g_printerr("Not pipeline element rtp could be created.\n");
        return -1;
    };
    if (!udpsink) {
        g_printerr("Not pipeline element updsink could be created.\n");
        return -1;
    };
    if (!tcpserver) {
        g_printerr("Not pipeline element updsink could be created.\n");
        return -1;
    };


    //gst_bin_add_many(GST_BIN(pipeline_v1), source, videotee, queue1, queue2, sink_app, enc, rtp,tcpserver, parser, NULL);
    //gst_bin_add_many(GST_BIN(pipeline_v1), source, videotee, queue1, queue2, sink_app, enc, rtp,udpsink, parser, NULL);
    //gst_bin_add_many(GST_BIN(pipeline_v1), source, videotee, queue1, queue2 , sink_app_1,fakesink,  NULL);
    gst_bin_add_many(GST_BIN(pipeline_v1), source,  queue1, sink_app_1,  NULL);

    if (!gst_element_link_many(source, queue1 , sink_app_1, NULL)) {
        gst_object_unref(pipeline_v1);        
        g_critical("Unable to link src to csp ");
        exit(1);
    }
/*
    if (!gst_element_link_many(source, videotee, NULL)) {
        gst_object_unref(pipeline_v1);        
        g_critical("Unable to link src to csp ");
        exit(1);
    }

    if (!gst_element_link_many(queue1, sink_app_1, NULL)) {
        printf("Cannot link gstreamer elements 1\n");
        exit(1);
    }

//    if (!gst_element_link_many(queue2, enc, parser, rtp, tcpserver, NULL)) { //enc,rtp, udpsink
//    if (!gst_element_link_many(queue2, enc, parser, rtp, udpsink, NULL)) { //enc,rtp, udpsink

    if (!gst_element_link_many(queue2, fakesink, NULL)) { //enc,rtp, udpsink
        printf("Cannot link gstreamer elements 2\n");
        exit(1);
    }

    GstPad *tee_q1_pad, *tee_q2_pad;
    GstPad *q1_pad, *q2_pad;

    tee_q1_pad = gst_element_get_request_pad(videotee, "src%d");
    g_print("Obtained request pad %s for q1 branch.\n", gst_pad_get_name(tee_q1_pad));
    q1_pad = gst_element_get_static_pad(queue1, "sink");

    if (gst_pad_link(tee_q1_pad, q1_pad) != GST_PAD_LINK_OK) {
        g_critical("Tee for q1 could not be linked.\n");
        gst_object_unref(pipeline_v1);        
        return 0;

    }


    tee_q2_pad = gst_element_get_request_pad(videotee, "src%d");
    g_print("Obtained request pad %s for q2 branch.\n", gst_pad_get_name(tee_q2_pad));
    q2_pad = gst_element_get_static_pad(queue2, "sink");


    if (gst_pad_link(tee_q2_pad, q2_pad) != GST_PAD_LINK_OK) {
        g_critical("Tee for q2 could not be linked.\n");
        gst_object_unref(pipeline_v1);        
        return 0;
    }
*/
  bus = gst_element_get_bus(pipeline_v1);

    
  /* create a server instance */

  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaMapping *mapping;
  GstRTSPMediaFactory *factory;
  server = gst_rtsp_server_new ();
  mapping = gst_rtsp_server_get_media_mapping (server);
  factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_shared(factory, TRUE);
  // appsrc name=mysrc imxv4l2src device=/dev/video0 fps-n=30 
  gst_rtsp_media_factory_set_launch (factory, "( appsrc name=mysrc ! vpuenc codec=6 ! rtph264pay name=pay0 pt=96  )");
  g_signal_connect (factory, "media-configure", G_CALLBACK (&media_configure), app);
  gst_rtsp_media_mapping_add_factory (mapping, "/videocam", factory);
  g_object_unref (mapping);
  gst_rtsp_server_attach (server,NULL);     


  loop = g_main_loop_new(NULL, FALSE);


   GstAppSinkCallbacks callbacks = { NULL, new_preroll, new_buffer,
                                      new_buffer_list, { NULL } };
   gst_app_sink_set_callbacks (GST_APP_SINK(sink_app_1), &callbacks, NULL, NULL);
    
   CustomData data;
   data.loop = loop;
   data.pipeline = pipeline_v1;
       
   guint bus_watch_id = gst_bus_add_watch(bus, bus_call, NULL);  
        
   ret = gst_element_set_state(pipeline_v1, GST_STATE_PLAYING);

   g_main_loop_run(loop);

/*
    gst_object_unref(tee_q1_pad);
    gst_object_unref(tee_q2_pad);
    gst_object_unref(q1_pad);
    gst_object_unref(q2_pad);
*/
    gst_object_unref(bus);

    gst_element_set_state(pipeline_v1, GST_STATE_NULL);
    gst_object_unref(pipeline_v1);

    cvReleaseImage(&m_RGB);

    delete buf_tmp;
	
    printf("Thread cancel\n");
    rc = pthread_cancel(thread);
    pthread_attr_destroy(&attr); 
    close(sockfd);
    pthread_exit(NULL);
    return 0;
}
