#ifndef ROS_PUBLISHER_H_
#define ROS_PUBLISHER_H_

#include <map>
#include <vector>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>

#include "ros_shim.h"   // de-ROS'd: rviz publishing is a no-op in the standalone build
#include "utils.h"
#include "read_configs.h"

enum FeatureMessgaeType {
  VOFeature = 0,
  RelocFeature = 1
};

struct FeatureMessgae{
  double time;
  cv::Mat image;
  cv::Mat key_image;
  int frame_id;
  int keyfrmae_id;
  std::vector<bool> inliers;
  std::vector<cv::KeyPoint> keyframe_keypoints;
  std::vector<cv::KeyPoint> keypoints;
  std::vector<Eigen::Vector4d> lines;
  std::vector<int> line_track_ids;
  std::vector<std::map<int, double>> points_on_lines;
  std::vector<cv::DMatch> matches;
  FeatureMessgaeType fm_type;
};
typedef std::shared_ptr<FeatureMessgae> FeatureMessgaePtr;
typedef std::shared_ptr<const FeatureMessgae> FeatureMessgaeConstPtr;

struct FramePoseMessage{
  double time;
  Eigen::Matrix4d pose;
};
typedef std::shared_ptr<FramePoseMessage> FramePoseMessagePtr;
typedef std::shared_ptr<const FramePoseMessage> FramePoseMessageConstPtr;

struct KeyframeMessage{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  double time;
  std::vector<double> times;
  std::vector<int> ids;
  std::vector<Eigen::Matrix4d> poses;
};
typedef std::shared_ptr<KeyframeMessage> KeyframeMessagePtr;
typedef std::shared_ptr<const KeyframeMessage> KeyframeMessageConstPtr;

struct MapMessage{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  double time;
  bool reset;
  std::vector<int> ids;
  std::vector<Eigen::Vector3d> points;
};
typedef std::shared_ptr<MapMessage> MapMessagePtr;
typedef std::shared_ptr<const MapMessage> MapMessageConstPtr;

struct MapLineMessage{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  double time;
  bool reset;
  std::vector<int> ids;
  std::vector<Vector6d> lines;
};
typedef std::shared_ptr<MapLineMessage> MapLineMessagePtr;
typedef std::shared_ptr<const MapLineMessage> MapLineMessageConstPtr;


struct RelocMessage{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  double map_scale;
  std::vector<double> times;
  std::vector<Eigen::Matrix4d> poses;
  std::vector<Eigen::Vector3d> mappoints;
};
typedef std::shared_ptr<RelocMessage> RelocMessagePtr;
typedef std::shared_ptr<const RelocMessage> RelocMessageConstPtr;


// De-ROS'd publisher: keeps the exact public API the SLAM core calls, but every
// method is a no-op. Live rviz visualization is dropped in the standalone build;
// trajectories are still written directly via Map*/Save* paths in the demos.
class RosPublisher{
public:
  RosPublisher(const RosPublisherConfig& ros_publisher_config, ros::NodeHandle nh);

  void PublishFeature(FeatureMessgaePtr feature_message);
  void PublishFramePose(FramePoseMessagePtr frame_pose_message);
  void PublisheKeyframe(KeyframeMessagePtr keyframe_message);
  void PublishMap(MapMessagePtr map_message);
  void PublishMapLine(MapLineMessagePtr mapline_message);
  void PubRelocResults(RelocMessagePtr reloc_message);

  void Clear();
  void ShutDown();

private:
  RosPublisherConfig _config;
};
typedef std::shared_ptr<RosPublisher> RosPublisherPtr;

#endif  // ROS_PUBLISHER_H_