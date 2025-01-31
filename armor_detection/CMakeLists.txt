cmake_minimum_required(VERSION 3.0.2)
project(armor_detection)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_module)
add_subdirectory(3rdparty/fmt)
add_subdirectory(3rdparty/nadjieb_mjpeg_streamer)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  roslib
  tf
  actionlib
  std_msgs
  roborts_msgs
  roborts_common
)
#find_package (nadjieb_mjpeg_streamer REQUIRED )
find_package(OpenCV 4 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(ProtoBuf REQUIRED)
catkin_package()

list(APPEND EXTRA_INCLUDES 3rdparty/fmt/include)
file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
rrts_protobuf_generate_cpp(${CMAKE_CURRENT_SOURCE_DIR}/proto
  ArmorDetectionProtoSrc
  ArmorDetectionProtoHds
  ${ProtoFiles})

# src directory
include_directories(${EIGEN3_INCLUDE_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR}/basic_pnp  DIR_SRCS_PNP)
aux_source_directory(${PROJECT_SOURCE_DIR}/basic_armor  DIR_SRCS_ARMOR)
aux_source_directory(${PROJECT_SOURCE_DIR}/uart_serial  DIR_SRCS_SERIAL)
aux_source_directory(${PROJECT_SOURCE_DIR}/mv_video_capture  DIR_SRCS_CAPTURE)


#server
add_executable(${PROJECT_NAME}_node
        armor_detection_node.cpp
        ${DIR_SRCS_PNP}
        ${DIR_SRCS_ARMOR}
        ${DIR_SRCS_SERIAL}
        ${DIR_SRCS_CAPTURE}
  	${ArmorDetectionProtoSrc}
  	${ArmorDetectionProtoHds}
 )
target_include_directories(armor_detection_node
        PRIVATE
        ${PROJECT_SOURCE_DIR}/basic_armor 
        ${PROJECT_SOURCE_DIR}/basic_pnp 
        ${PROJECT_SOURCE_DIR}/mv_video_capture 
        ${PROJECT_SOURCE_DIR}/uart_serial 
        ${catkin_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRECTORIES}
)
target_link_libraries(armor_detection_node
        PUBLIC
        ${catkin_LIBRARIES}
        ${OpenCV_LIBS}
	    ${EXTRA_LIBS}
	    ${PROTOBUF_LIBRARIES}
        /lib/libMVSDK.so
        fmt::fmt
        PRIVATE nadjieb_mjpeg_streamer::nadjieb_mjpeg_streamer
)


# clinet 
add_executable(${PROJECT_NAME}_client    armor_detection_client.cpp)
target_include_directories(armor_detection_client PRIVATE  ${catkin_INCLUDE_DIRS})
target_link_libraries(armor_detection_client  PRIVATE   ${catkin_LIBRARIES} )

add_dependencies(armor_detection_node 
  roborts_msgs_generate_messages)
add_dependencies(armor_detection_client 
  roborts_msgs_generate_messages)

