#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include "ros_shim.h"

#include "utils.h"
#include "read_configs.h"
#include "map.h"
#include "map_refiner.h"

// Standalone (de-ROS'd) usage:
//   map_refinement <config_path> <model_dir> <map_root> <voc_path> [breakpoint]
int main(int argc, char **argv) {
  ros::init(argc, argv, "air_slam");
  ros::NodeHandle nh;

  if (argc < 5) {
    std::cout << "usage: map_refinement <config_path> <model_dir> <map_root> "
                 "<voc_path> [breakpoint]" << std::endl;
    return 1;
  }
  int breakpoint = (argc > 5) ? std::atoi(argv[5]) : 0;

  std::string config_path = argv[1];
  std::string model_dir = argv[2];
  MapRefinementConfigs configs(config_path, model_dir);
  MapRefiner map_refiner(configs, nh);

  std::string map_root = argv[3];
  std::cout << "Loading map and vocabulary..." << std::endl;
  map_refiner.LoadMap(map_root);

  std::string voc_path = argv[4];
  map_refiner.LoadVocabulary(voc_path);
  std::cout << "Done." << std::endl;

  map_refiner.Wait(breakpoint);

  std::cout << "Building covisibility graph..." << std::endl;
  map_refiner.UpdateCovisibilityGraph();
  std::cout << "Done." << std::endl;

  std::cout << "Loop detection..." << std::endl;
  int loop_num = map_refiner.LoopDetection();
  std::cout << "Done, " << loop_num << " loop pairs are found." << std::endl;

  std::cout << "Optimizing pose graph..." << std::endl;
  map_refiner.PoseGraphRefinement();
  std::cout << "Done." << std::endl;

  map_refiner.Wait(breakpoint);

  std::cout << "Merging mappoints..." << std::endl;
  map_refiner.MergeMap();
  std::cout << "Done." << std::endl;

  map_refiner.Wait(breakpoint);

  std::cout << "Optimizing global map..." << std::endl;
  map_refiner.GlobalMapOptimization();
  map_refiner.UpdateCovisibilityGraph();
  std::cout << "Done." << std::endl;

  std::cout << "Build junction database..." << std::endl;
  map_refiner.BuildJunctionDatabase();
  std::cout << "Done." << std::endl;

  std::string trajectory_global_ba_path = ConcatenateFolderAndFileName(map_root, "trajectory_v1.txt");
  map_refiner.SaveTrajectory(trajectory_global_ba_path);

  map_refiner.Wait(breakpoint);

  std::cout << "Saving final map..." << std::endl;
  map_refiner.SaveFinalMap(map_root);
  std::cout << "Done." << std::endl;

  exit(0);
  map_refiner.StopVisualization();
  ros::shutdown();

  return 0;
}
