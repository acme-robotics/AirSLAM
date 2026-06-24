#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include "ros_shim.h"
#include <thread>

#include "read_configs.h"
#include "dataset.h"
#include "map_builder.h"
#include "debug.h"

#include "plnet.h"
#include "feature_detector.h"

// Standalone (de-ROS'd) usage:
//   test_feature <camera_config_path> <dataroot> <model_dir> <save_root>
int main(int argc, char **argv) {
  ros::init(argc, argv, "air_slam");

  if (argc < 5) {
    std::cout << "usage: test_feature <camera_config_path> <dataroot> "
                 "<model_dir> <save_root>" << std::endl;
    return 1;
  }
  std::string camera_config_path = argv[1];
  std::string dataroot = argv[2];
  std::string model_dir = argv[3];
  std::string save_root = argv[4];

  MakeDir(save_root);

  PLNetConfig plnet_config;
  plnet_config.use_superpoint = 1;
  plnet_config.max_keypoints = 400;
  plnet_config.keypoint_threshold = 0.04;
  plnet_config.remove_borders = 4;
  plnet_config.line_threshold = 0.5;
  plnet_config.line_length_threshold = 50;
  plnet_config.SetModelPath(model_dir);

  CameraPtr _camera = std::shared_ptr<Camera>(new Camera(camera_config_path));

  FeatureDetectorPtr feature_detector = std::shared_ptr<FeatureDetector>(new FeatureDetector(plnet_config));

  std::vector<std::string> image_names;
  GetFileNames(dataroot, image_names);
  size_t dataset_length = image_names.size();
  // dataset_length = 14;
  for(size_t i = 0; i < dataset_length && ros::ok(); ++i){
    std::cout << "i ====== " << i << std::endl;

    std::string image_path = ConcatenateFolderAndFileName(dataroot, image_names[i]);
    cv::Mat image = cv::imread(image_path, 0);

    cv::Mat image_left_rect;
    _camera->UndistortImage(image, image_left_rect);

    Eigen::Matrix<float, 259, Eigen::Dynamic> features;
    std::vector<float> line_scores;
    std::vector<Eigen::Vector4d> lines;

    auto before_infer = std::chrono::high_resolution_clock::now();

    cv::Mat resized_image;
    cv::resize(image_left_rect, resized_image, cv::Size(512, 512));

    // feature_detector->Detect(image_left_rect, features);
    feature_detector->Detect(image_left_rect, features, lines);

    SaveLineDetectionResult(image_left_rect, lines, save_root, std::to_string(i));
    // SaveDetectorResult(image_left_rect, features, save_root, std::to_string(i));

    std::cout << "lines ====== " << lines.size() << std::endl;

    auto after_infer = std::chrono::high_resolution_clock::now();
    auto cost_time = std::chrono::duration_cast<std::chrono::milliseconds>(after_infer - before_infer).count();
    std::cout << "One Frame Processinh Time: " << cost_time << " ms." << std::endl;
  }


  ros::shutdown();

  return 0;
}
