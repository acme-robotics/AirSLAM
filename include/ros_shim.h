#ifndef ROS_SHIM_H_
#define ROS_SHIM_H_

// Minimal stand-ins for the handful of ROS symbols AirSLAM's core uses, so the
// standalone (non-catkin) build needs no ROS at all. See AIRSLAM.md.
//
// AirSLAM only ever touches ROS through: a NodeHandle passed (unused) into the
// MapBuilder/MapRefiner/MapUser constructors, init/ok/spinOnce/shutdown for the
// process lifecycle, and Time/Rate in the (now no-op) visualization loops. None
// of those need a running roscore; we replace them with header-only stubs.
//
//   - ros::ok() returns true forever: every loop that consults it is also bounded
//     by the dataset length or an explicit _stop flag, so this never spins free.
//   - ros::Time::now() is a real monotonic clock (steady_clock); the only callers
//     feed it to the no-op publisher as a message stamp.

#include <chrono>
#include <string>
#include <thread>

namespace ros {

struct NodeHandle {};

inline void init(int /*argc*/, char** /*argv*/, const std::string& /*name*/) {}
inline bool ok() { return true; }
inline void spinOnce() {}
inline void shutdown() {}

struct Time {
  double t;
  Time() : t(0.0) {}
  explicit Time(double s) : t(s) {}
  double toSec() const { return t; }
  static Time now() {
    using namespace std::chrono;
    return Time(duration_cast<duration<double>>(
                    steady_clock::now().time_since_epoch())
                    .count());
  }
};

struct Duration {
  double t;
  explicit Duration(double s = 0.0) : t(s) {}
  void sleep() const {
    std::this_thread::sleep_for(std::chrono::duration<double>(t));
  }
};

struct Rate {
  std::chrono::duration<double> period;
  explicit Rate(double hz) : period(hz > 0.0 ? 1.0 / hz : 0.0) {}
  void sleep() { std::this_thread::sleep_for(period); }
};

}  // namespace ros

#endif  // ROS_SHIM_H_
