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
#include "Counter.h"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

GMainLoop *loop;
typedef struct _CustomData {
    gboolean is_live;
    GstElement *pipeline;
    GMainLoop *loop;
} CustomData;

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
    GstBuffer *buffer =  gst_app_sink_pull_preroll (sink);
    if (buffer) {
        print_buffer(buffer, "preroll");
	gst_buffer_unref(buffer);
   }
   return GST_FLOW_OK;
}

IplImage *frame = NULL;
IplImage *m_RGB = NULL;
unsigned char *buf_tmp = NULL;
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

static GstFlowReturn new_buffer(GstAppSink *sink, gpointer user_data) {
    int height =480; //480;//1944;
    int width = 640;//640;//2592;
    static int  cnt=0;

    GstBuffer* buffer;

//    printf("# %d #\n",cnt++);    
    g_mutex_lock(&mutex);
    buffer =  gst_app_sink_pull_buffer (sink);
    if (buffer) {
//            printf("size %d \n",GST_BUFFER_SIZE(buffer));
            unsigned char *pData=(unsigned char*)GST_BUFFER_DATA(buffer);
            if (m_RGB==NULL) m_RGB = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
            if (buf_tmp ==NULL) buf_tmp=new unsigned char[width*height*3];
            memcpy((void*)buf_tmp,(void*)pData,width*height*1.5);
            convert(buf_tmp, m_RGB->imageData, height * width);
	    cv::Mat frame=cv::Mat(480,640,CV_8UC3,m_RGB->imageData);
//    	    int currentCount = cnter->processFrame(frame);
	    sprintf(name_file,"cap%d.bmp",cnt++);
 	    cv::Mat mat_img(m_RGB);
//          cv::imwrite(name_file, mat_img);
//          mat_img.release();
//          cvReleaseImage(&m_RGB);

        gst_buffer_unref(buffer);
//        printf("\n");
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
            // 	exit(1);
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
    case SIGINT:
      fputs("Caught SIGINT: interactive attention signal, probably a ctrl+c!!!!\n",
            stderr);
      g_main_loop_quit(loop);
      return ;
      break;
    case SIGSEGV:
      fputs("Caught SIGSEGV: segfault\n", stderr);
      break;
    case SIGTERM:
    default:
      fputs("Caught SIGTERM: a termination request was sent to the program\n",
            stderr);
      break;
  }
  _Exit(1);
}


int main(int argc, char *argv[]) {

    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    GstElement *pipeline_v1, *pipeline_v2;
    GstElement *source, *sink_app, *sink_file, *csp, *enc, *videotee, *parser;
    GstElement *queue1, *queue2;
    GstElement *rtp, *udpsink, *fakesink;

    /* Initialize GStreamer */
    signal(SIGABRT, almost_c99_signal_handler);
    signal(SIGFPE,  almost_c99_signal_handler);
    signal(SIGILL,  almost_c99_signal_handler);
    signal(SIGINT,  almost_c99_signal_handler);
    signal(SIGSEGV, almost_c99_signal_handler);
    signal(SIGTERM, almost_c99_signal_handler);
    
    gst_init(&argc, &argv);
    std::string pathToConfig = "confZone1.txt";
    init_counting(pathToConfig);

    printf("Start gstreamer 0.1 and Counting\n");	

    source = gst_element_factory_make("imxv4l2src", "source");
    
    queue1 = gst_element_factory_make("queue", "queue1");
    queue2 = gst_element_factory_make("queue", "queue2");
//  g_object_set( G_OBJECT(queue1), "max-size-buffers", 10, NULL);
        
     enc = gst_element_factory_make("vpuenc", "imxvpu");
     g_object_set(G_OBJECT(enc), "codec", 6, NULL);

    parser = gst_element_factory_make("h264parse", "h264parse");
    
    rtp = gst_element_factory_make("rtph264pay", "rtp");

    fakesink = gst_element_factory_make("fakesink", "fakesink");
    
    udpsink = gst_element_factory_make("udpsink", "updsink");
    
    g_object_set(G_OBJECT(udpsink), "host", "10.0.0.2", NULL);
    g_object_set(G_OBJECT(udpsink), "port", 5000, NULL);
    g_object_set(G_OBJECT(udpsink), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(udpsink), "async", FALSE, NULL);
    
    sink_app = gst_element_factory_make("appsink", "appsink");
    
    sink_file = gst_element_factory_make("filesink", "filesink");
    g_object_set(G_OBJECT(sink_file), "location", "t1.jpeg", NULL);
    
    videotee = gst_element_factory_make("tee", "videotee");

    g_object_set(G_OBJECT(source), "device", "/dev/video0", NULL);
    //  g_object_set( G_OBJECT(source), "num-buffers", "-1", NULL);
    //  g_object_set( G_OBJECT(source), "capture-mode", 0, NULL); 
      g_object_set( G_OBJECT(source), "fps-n", "30", NULL); 



    pipeline_v1 = gst_pipeline_new("cam-pipeline");
    pipeline_v2 = gst_pipeline_new("cam-pipeline");


    if (!pipeline_v1 || !pipeline_v2 || !source || !sink_app || !sink_file || !queue1 || !queue2 || !videotee || !enc || !parser) {
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

    gst_bin_add_many(GST_BIN(pipeline_v1), source, videotee, queue1, queue2, sink_app, enc, rtp, udpsink, parser, NULL);


    if (!gst_element_link_many(source, videotee, NULL)) {
        gst_object_unref(pipeline_v1);        
        g_critical("Unable to link src to csp ");
        exit(1);
    }

    if (!gst_element_link_many(queue1, sink_app, NULL)) {
        printf("Cannot link gstreamer elements 1\n");
        exit(1);
    }

    if (!gst_element_link_many(queue2, enc, parser, rtp, udpsink, NULL)) { //enc,rtp,
        printf("Cannot link gstreamer elements 2\n");
        exit(1);
    }


    GstPad *tee_q1_pad, *tee_q2_pad;
    GstPad *q1_pad, *q2_pad;

    tee_q1_pad = gst_element_get_request_pad(videotee, "src%d");
    g_print("Obtained request pad %s for q1 branch.\n", gst_pad_get_name(tee_q1_pad));
    q1_pad = gst_element_get_static_pad(queue1, "sink");

    tee_q2_pad = gst_element_get_request_pad(videotee, "src%d");
    g_print("Obtained request pad %s for q2 branch.\n", gst_pad_get_name(tee_q2_pad));
    q2_pad = gst_element_get_static_pad(queue2, "sink");

    if (gst_pad_link(tee_q1_pad, q1_pad) != GST_PAD_LINK_OK) {

        g_critical("Tee for q1 could not be linked.\n");
        gst_object_unref(pipeline_v1);
        //gst_object_unref (pipeline_v2);
        return 0;

    }


    if (gst_pad_link(tee_q2_pad, q2_pad) != GST_PAD_LINK_OK) {

        g_critical("Tee for q2 could not be linked.\n");
        gst_object_unref(pipeline_v1);
        // gst_object_unref (pipeline_v2);
        return 0;
    }


    bus = gst_element_get_bus(pipeline_v1);

    
    loop = g_main_loop_new(NULL, FALSE);
        
    GstAppSinkCallbacks callbacks = { NULL, new_preroll, new_buffer,
                                      new_buffer_list, { NULL } };
    gst_app_sink_set_callbacks (GST_APP_SINK(sink_app), &callbacks, NULL, NULL);
    
    CustomData data;
    data.loop = loop;
    data.pipeline = pipeline_v1;

    
    gst_app_sink_set_callbacks(GST_APP_SINK(sink_app), &callbacks, NULL, NULL);
    g_signal_connect (udpsink, "client-added", G_CALLBACK (&add_cliden), NULL);
    
    guint bus_watch_id = gst_bus_add_watch(bus, bus_call, NULL);  
        
    ret = gst_element_set_state(pipeline_v1, GST_STATE_PLAYING);



    g_main_loop_run(loop);

    gst_object_unref(tee_q1_pad);
    gst_object_unref(tee_q2_pad);
    gst_object_unref(q1_pad);
    gst_object_unref(q2_pad);
    gst_object_unref(bus);

    gst_element_set_state(pipeline_v1, GST_STATE_NULL);
    gst_object_unref(pipeline_v1);

    cvReleaseImage(&m_RGB);
    delete buf_tmp;

    return 0;
}
