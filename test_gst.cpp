#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
//#include <gst/app/gstappsrc.h>
//#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>  
#include <opencv2/opencv.hpp>

GMainLoop *loop;

void print_buffer(GstAppSink *sink, const char *title);

static void convert(unsigned char *src,char *dst,int size) {
	for(int i = 0, j=0; i < size * 3; i+=6, j+=4)  //+2
	{

//	    m_RGB->imageData[i] = pData[j+0] ;
	    dst[i] = src[j+0] ;
	    dst[i+1] =src[j+0];
	    dst[i+2] = src[j+0];
	    dst[i+3] = src[j+2];
	    dst[i+4] = src[j+2];
	    dst[i+5] = src[j+2];
	}
};

static void new_oes (GstAppSink *sink, gpointer user_data)
{ 
      printf("###### eos #######\n");	
//    GstBufferList *list = gst_app_sink_pull_buffer_list (sink);
//    GstBufferListIterator *it = gst_buffer_list_iterate (list);
//    GstBuffer *buffer;
//    while (gst_buffer_list_iterator_next_group (it))
//        while ((buffer = gst_buffer_list_iterator_next (it)) != NULL) print_buffer(buffer, "new_buffer_list");
//    gst_buffer_list_iterator_free (it);
 
 
}
 
static GstFlowReturn new_preroll (GstAppSink *sink, gpointer user_data)
{
     printf("#####  new_preroll #######!!!");	
     GstSample *buffer =  gst_app_sink_pull_preroll (sink);
     if (buffer) {
	print_buffer(sink, "preroll");
	gst_sample_unref (buffer);
     }
    return GST_FLOW_OK;
}

IplImage *frame = NULL;
IplImage *m_RGB = NULL;

static GstFlowReturn new_buffer(GstAppSink *sink, gpointer user_data)
{
   int height=480;
   int width=640;
   GstMapInfo map;

    GstSample* sample;
    GstBuffer* buffer;
    GstCaps* caps;
    printf("#");
    sample =  gst_app_sink_pull_sample(sink);
//    frame = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U,2);	
    //m_RGB = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);

    if (sample) {
      buffer = gst_sample_get_buffer(sample);
      caps = gst_sample_get_caps(sample);
      gst_buffer_map (buffer, &map, GST_MAP_READ);
        printf("size = %d  ",map.size);
        unsigned char *pData = (unsigned char*)map.data;
      	m_RGB = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
	convert(pData,m_RGB->imageData,height*width);
/*
	for(int i = 0, j=0; i < width*height * 3; i+=6, j+=4)  //+2
	{

	    m_RGB->imageData[i] = pData[j+0] ;//+ pData[j+3]*((1 - 0.299)/0.615);
	    m_RGB->imageData[i+1] = pData[j+0];// - pData[j+1]*((0.114*(1-0.114))/(0.436*0.587)) - pData[j+3]*((0.299*(1 - 0.299))/(0.615*0.587));
	    m_RGB->imageData[i+2] = pData[j+0];// + pData[j+1]*((1 - 0.114)/0.436);
	    m_RGB->imageData[i+3] = pData[j+2];// + pData[j+3]*((1 - 0.299)/0.615);
	    m_RGB->imageData[i+4] = pData[j+2];// - pData[j+1]*((0.114*(1-0.114))/(0.436*0.587)) - pData[j+3]*((0.299*(1 - 0.299))/(0.615*0.587));
	    m_RGB->imageData[i+5] = pData[j+2];// + pData[j+1]*((1 - 0.114)/0.436);
	    //printf("%d %d %d %d\n",pData[j],pData[j+1],pData[j+2],pData[j+3]);
	}
*/
      cv::Mat mat_img(m_RGB);
      cv::imwrite( "my_bitmap.bmp", mat_img);		
      gst_buffer_unmap (buffer, &map);
      gst_buffer_unref(buffer);
     printf("\n",map.size);
    }
/*
   frame = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U,2);	
   m_RGB = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);

   printf("####	app buffer   ####\n");	
   GstSample *sample =  gst_app_sink_pull_sample(sink);
   GstBuffer *buffer =  gst_sample_get_buffer (sample);
   gst_buffer_map (buffer, &map, GST_MAP_READ);
   char *pData=new char[480*640*3];
   if (buffer) print_buffer(sink, "buffer"); else printf("NULL new buffer");

  cv::Mat mat_img(m_RGB);
  cv::imwrite( "my_bitmap.bmp", mat_img);		
  gst_buffer_unmap (buffer, &map);
  gst_sample_unref (sample);
  gst_buffer_unref (buffer);
*/
    return GST_FLOW_OK;
}

void print_buffer (GstAppSink *sink, const char *title)
{
    GstCaps *caps = gst_app_sink_get_caps(sink);
    for (uint j = 0; j < gst_caps_get_size(caps); ++j) {
        GstStructure *structure = gst_caps_get_structure(caps, j);
        printf("%s{%ss}: ",(title),(gst_structure_get_name(structure)));
        for (int i = 0; i < gst_structure_n_fields(structure); ++i) {
            if (i != 0)
                printf(", ");
            const char *name = gst_structure_nth_field_name(structure, i);
            GType type = gst_structure_get_field_type(structure, name);
            printf("%s[%s]",(name),(g_type_name(type)));
        }
        printf("\n");
    }
}



static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer user_data)
{
    int i;

    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_STATE_CHANGED: {
        GstState old_state, new_state;
 
        gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);
	printf("[%s]: %s -> %s\n", GST_OBJECT_NAME (msg->src),gst_element_state_get_name (old_state),gst_element_state_get_name (new_state));
        break;
    }
    case GST_MESSAGE_ERROR: {
        gchar *debug;
        GError *err;
 
        gst_message_parse_error (msg, &err, &debug);
	printf("[%s]: %s %s\n", GST_OBJECT_NAME (msg->src), err->message, debug);
        g_free (debug);
        g_error_free (err);
// 	exit(1);
        break;
    }
   case GST_MESSAGE_EOS:
	/* end-of-stream */
	g_main_loop_quit (loop);
   break;
   default: {
	const GstStructure *structure = gst_message_get_structure(msg);
        if (structure) {
                printf("%s{%s} ",gst_message_type_get_name (msg->type),gst_structure_get_name(structure));
            for (i = 0; i < gst_structure_n_fields(structure); ++i) {
                if (i != 0) printf(", ");
                const char *name = gst_structure_nth_field_name(structure, i);
                GType type = gst_structure_get_field_type(structure, name);
                printf("%s",name);
                printf("[%s]",g_type_name(type));
            }
            printf("\n");
        } else {
            printf("%s{}\n", gst_message_type_get_name (msg->type));
        }

        break;
    }
    }
    return true;
}

int main(int argc, char *argv[]) {

  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  GstElement *pipeline, *source, *sink_app, *sink_file ,*csp,*queue,*enc;
  /* Initialize GStreamer */
  gst_init (&argc, &argv);


  source = gst_element_factory_make ("v4l2src", "source");
  queue = gst_element_factory_make ("queue", "queue");
  enc	= gst_element_factory_make ("jpegenc", "jpegenc");
  sink_app = gst_element_factory_make ("appsink", "appsink");
  sink_file = gst_element_factory_make ("filesink", "filesink");
  g_object_set( G_OBJECT(queue), "max-size-buffers", 10, NULL);

  g_object_set( G_OBJECT(source), "device", "/dev/video0", NULL);
//  g_object_set( G_OBJECT(source), "num-buffers", "-1", NULL);
//  g_object_set( G_OBJECT(source), "capture-mode", 0, NULL); 
//  g_object_set( G_OBJECT(source), "fps-n", "30", NULL); 

   

  pipeline = gst_pipeline_new ("cam-pipeline");


  gst_bin_add_many (GST_BIN (pipeline), source,sink_app, NULL);


  if (!pipeline) {
    g_printerr ("Not pipeline element could be created.\n");
    return -1;
  }


  if ( !source) {
    g_printerr ("Not source element could be created.\n");
    return -1;
  }
   
  if (!sink_app) {
    g_printerr ("Not sink element could be created.\n");
    return -1;
  }

  if (!sink_file) {
    g_printerr ("Not sink element could be created.\n");
    return -1;
  }

  if (!queue) {
    g_printerr ("Not ffmpegcolorspace element could be created.\n");
    return -1;
  }

   if(!gst_element_link_many (source,sink_app, NULL)) {
         printf("Cannot link gstreamer elements");
         exit (1);
    }

  bus = gst_element_get_bus (pipeline); 


  GstAppSinkCallbacks callbacks;
  callbacks.eos=&new_oes;
  callbacks.new_preroll=&new_preroll;
  callbacks.new_sample=&new_buffer;

   gst_app_sink_set_callbacks (GST_APP_SINK(sink_app), &callbacks, NULL, NULL);
//    g_object_set (sink_app, "emit-signals", TRUE, NULL);
//    g_signal_connect (sink_app, "new-sample", G_CALLBACK (new_buffer), NULL);

  guint  bus_watch_id = gst_bus_add_watch (bus, bus_call, NULL);

  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);


  loop = g_main_loop_new (NULL, FALSE);    
  g_main_loop_run(loop);


  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);


  return 0;
}
