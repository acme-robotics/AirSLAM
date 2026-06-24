// De-ROS'd publisher implementation: no-op bodies for the standalone build.
//
// The original published features/poses/keyframes/map/maplines/reloc results to
// rviz over ROS topics. The standalone build has no ROS and runs headless, so
// every method here does nothing. Trajectory output is unaffected: the demo
// mains write it directly through MapBuilder::SaveTrajectory / SaveTumTrajectory.
#include "ros_publisher.h"

RosPublisher::RosPublisher(const RosPublisherConfig& ros_publisher_config,
                           ros::NodeHandle /*nh*/)
    : _config(ros_publisher_config) {}

void RosPublisher::PublishFeature(FeatureMessgaePtr /*feature_message*/) {}
void RosPublisher::PublishFramePose(FramePoseMessagePtr /*frame_pose_message*/) {}
void RosPublisher::PublisheKeyframe(KeyframeMessagePtr /*keyframe_message*/) {}
void RosPublisher::PublishMap(MapMessagePtr /*map_message*/) {}
void RosPublisher::PublishMapLine(MapLineMessagePtr /*mapline_message*/) {}
void RosPublisher::PubRelocResults(RelocMessagePtr /*reloc_message*/) {}

void RosPublisher::Clear() {}
void RosPublisher::ShutDown() {}
