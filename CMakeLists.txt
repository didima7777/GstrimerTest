project (Tutorial)

set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
#set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_flann -lopencv_features2d -lopencv_calib3d -lopencv_video")		
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgobject-2.0 -lgmodule-2.0 -pthread -lgthread-2.0 -pthread -lglib-2.0 -lxml2")		
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgstreamer-1.0 -lgstapp-1.0 -lgstbase-1.0")  
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgstreamer-0.10 -lgstapp-0.10 -lgstbase-0.10")  
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -fpermissive")


include_directories(/usr/src/kernel/include/uapi)
include_directories(/usr/src/kernel/include)
include_directories(/usr/include/gstreamer-1.0)
include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib/x86_64-linux-gnu/glib-2.0/include/)
include_directories(/usr/lib/glib-2.0/include)
include_directories(/usr/include/libxml2)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

set  (APP_NAME test_gst)

#set (HEADERS  myBackgroundSubtracorMOG.h v4lwrapper.h bmp.h Counter.h)

list(APPEND SRC  test_gst.cpp)
#list(APPEND SRC  Variscite_test.cpp )
#list(APPEND SRC  myBackgroundSubtracorMOG.cpp)
#list(APPEND SRC  v4lwrapper.cpp)
#list(APPEND SRC  bmp.cpp)
#list(APPEND SRC  Counter.cpp)

add_executable(${APP_NAME} ${HEADERS} ${SRC})
target_link_libraries(${APP_NAME} ${OpenCV_LIBS} )



