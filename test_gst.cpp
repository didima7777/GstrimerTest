#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gst/gst.h>
//#include <gst/app/gstappsrc.h>
//#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>  
#include <opencv2/opencv.hpp>

void print_buffer (GstBuffer *buffer, const char *title);

static GstFlowReturn new_buffer_list (GstAppSink *sink, gpointer user_data)
{ 
//    GstBufferList *list = gst_app_sink_pull_buffer_list (sink);
//    GstBufferListIterator *it = gst_buffer_list_iterate (list);
    GstBuffer *buffer;
//    while (gst_buffer_list_iterator_next_group (it))
//        while ((buffer = gst_buffer_list_iterator_next (it)) != NULL) print_buffer(buffer, "new_buffer_list");
//    gst_buffer_list_iterator_free (it);
 
    return GST_FLOW_OK;
}
 
static GstFlowReturn new_preroll (GstAppSink *sink, gpointer user_data)
{
//    GstBuffer *buffer =  gst_app_sink_pull_preroll (sink);
//    if (buffer) print_buffer(buffer, "preroll");


    return GST_FLOW_OK;
}

IplImage *frame = NULL;
IplImage *m_RGB = NULL;

static GstFlowReturn new_buffer(GstAppSink *sink, gpointer user_data)
{
   int height=480;
   int width=640;
 
/*
	for(int i = 0, j=0; i < width*height * 3; i+=6, j+=2)  //+2
	{
	    m_RGB->imageData[i] = pData[j+0] ;
	    m_RGB->imageData[i+1] = pData[j+0];
	    m_RGB->imageData[i+2] = pData[j+0];
	    m_RGB->imageData[i+3] = pData[j+1];
	    m_RGB->imageData[i+4] = pData[j+1];
	    m_RGB->imageData[i+5] = pData[j+1];
	}

	cv::Mat mat_img(m_RGB);
	cv::imwrite( "my_bitmap.bmp", mat_img);		
*/

    return GST_FLOW_OK;
}

void print_buffer (GstBuffer *buffer, const char *title)
{
}
  GstElement *pipeline, *source, *sink,*csp,*queue,*enc;

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
	printf("!!! [%s]: %s %s\n", GST_OBJECT_NAME (msg->src), err->message, debug);
        g_free (debug);
        g_error_free (err);
 	exit(1);
        break;
    }
    default: {
/*
        const GstStructure *structure = msg->structure;
        if (structure) {
                printf("%s{%s}",gst_message_type_get_name (msg->type),gst_structure_get_name(structure));
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
*/
        break;
    }
    }
    return true;
}

int main(int argc, char *argv[]) {

  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
/*   

  source = gst_element_factory_make ("v4l2src", "source");
  queue = gst_element_factory_make ("queue", "queue");
  enc	= gst_element_factory_make ("jpegenc", "jpegenc");
  sink = gst_element_factory_make ("appsink", "appsink");
//  sink = gst_element_factory_make ("filesink", "filesink");
  g_object_set( G_OBJECT(queue), "max-size-buffers", 3, NULL);

//  g_object_set( G_OBJECT(source), "device", "/dev/video0", NULL);
//  g_object_set( G_OBJECT(source), "num-buffers", "-1", NULL);
  g_object_set( G_OBJECT(source), "capture-mode", 0, NULL); 
  g_object_set( G_OBJECT(source), "fps-n", "30", NULL); 

   

  pipeline = gst_pipeline_new ("cam-pipeline");


  gst_bin_add_many (GST_BIN (pipeline), source,queue,sink, NULL);


  if (!pipeline) {
    g_printerr ("Not pipeline element could be created.\n");
    return -1;
  }


  if ( !source) {
    g_printerr ("Not source element could be created.\n");
    return -1;
  }
   
  if (!sink) {
    g_printerr ("Not sink element could be created.\n");
    return -1;
  }

  if (!queue) {
    g_printerr ("Not ffmpegcolorspace element could be created.\n");
    return -1;
  }

   if(!gst_element_link_many (source,queue,sink, NULL)) {
         printf("Cannot link gstreamer elements");
         exit (1);
    }
  bus = gst_element_get_bus (pipeline);

  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);


   GMainLoop *loop = g_main_loop_new (NULL, FALSE);    
   g_main_loop_run(loop);


  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

*/
  return 0;
}
