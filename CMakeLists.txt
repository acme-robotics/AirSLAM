cmake_minimum_required(VERSION 3.5)
project(air_slam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "release")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
add_definitions(-w)

# Our g2o (airslam-build.sh) is built with its vendored Ceres (G2O_USE_VENDORED_CERES).
# g2o's installed base_fixed_sized_edge.h gates the ceres include on that macro *before*
# including g2o/config.h, and our FindG2O module (unlike g2o's own CMake config) doesn't
# propagate it -- so define it here, else the header falls back to a system <ceres/...>
# that we don't have. The vendored header ships under $G2O_ROOT/include/g2o/EXTERNAL/.
add_definitions(-DG2O_USE_VENDORED_CERES)

# --- Standalone (de-ROS'd) build -------------------------------------------------
# Upstream AirSLAM is a catkin/ROS-Noetic package. This fork (n6-orin-patches)
# drops the thin ROS layer (param-load -> argv in demo/, ros_publisher -> no-op,
# rviz dropped) so it builds as a plain CMake project like our other SLAM peers
# (ORB_SLAM3/SP_SLAM3/DBA-Fusion). gflags/glog are not used by the code and were
# removed; the only non-system dep we build ourselves is g2o (see airslam-build.sh,
# installed to $G2O_ROOT, default ~/.local). See AIRSLAM.md.

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/tensorrtbuffer)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/DBoW2)

find_package(OpenCV 4 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(CUDA REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(G2O REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
  ${CUDA_INCLUDE_DIRS}
  ${YAML_CPP_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${G2O_INCLUDE_DIR}
)

add_library(${PROJECT_NAME}_lib SHARED
  src/g2o_optimization/vertex_line3d.cc
  src/g2o_optimization/vertex_vi_pose.cc
  src/g2o_optimization/vertex_imu.cc
  src/g2o_optimization/edge_project_point.cc
  src/g2o_optimization/edge_project_line.cc
  src/g2o_optimization/edge_imu.cc
  src/g2o_optimization/edge_relative_pose.cc
  src/g2o_optimization/g2o_optimization.cc
  src/bow/FSuperpoint.cc
  src/bow/database.cc
  src/super_point.cpp
  src/feature_detector.cc
  src/super_glue.cpp
  src/light_glue.cpp
  src/plnet.cpp
  src/utils.cc
  src/camera.cc
  src/imu.cc
  src/dataset.cc
  src/frame.cc
  src/point_matcher.cc
  src/mappoint.cc
  src/mapline.cc
  src/line_processor.cc
  src/ros_publisher.cc
  src/map.cc
  src/map_builder.cc
  src/map_refiner.cc
  src/map_user.cc
  src/timer.cc
  src/debug.cc
)

target_link_libraries(${PROJECT_NAME}_lib
  nvinfer
  nvonnxparser
  ${OpenCV_LIBRARIES}
  ${CUDA_LIBRARIES}
  ${Boost_LIBRARIES}
  ${G2O_LIBRARIES}
  yaml-cpp
  tensorrtbuffer
  DBoW2
)

add_executable(visual_odometry demo/visual_odometry.cpp)
target_link_libraries(visual_odometry ${PROJECT_NAME}_lib)

add_executable(map_refinement demo/map_refinement.cpp)
target_link_libraries(map_refinement ${PROJECT_NAME}_lib)

add_executable(relocalization demo/relocalization.cpp)
target_link_libraries(relocalization ${PROJECT_NAME}_lib)

add_executable(test_feature demo/test_feature.cpp)
target_link_libraries(test_feature ${PROJECT_NAME}_lib)