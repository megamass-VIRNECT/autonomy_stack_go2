#include "rclcpp/rclcpp.hpp"
#include <iostream>

#include "common/ros2_sport_client.h"
#include "unitree_api/msg/request.hpp"

#include "unitree_go/msg/sport_mode_state.hpp"
#include <sensor_msgs/msg/joy.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <std_msgs/msg/bool.hpp> // Added for control_mode topic

// rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr state_suber;
// rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr vel_cmd_suber;
rclcpp::Publisher<unitree_api::msg::Request>::SharedPtr req_puber;
rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_suber;
rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr control_mode_suber; // Added control_mode subscriber

unitree_api::msg::Request req;
SportClient sport_req;
bool new_cmd = false;
bool manual_control_enabled_ = true; // Added flag for manual control, true by default

float vx;
float vyaw;
float vy;
int count_down = 0;

rclcpp::Node::SharedPtr nh;

bool checkObstacle = true;
float joySpeed = 0;
float joySpeedRaw = 0;
float joySpeedYaw = 0;
float joySpeedLateral = 0;

float PI = 3.141592653589397;
float maxSpeedYaw = 1.4;
float maxSpeedLateral = 0.5;

void controlModeCallback(const std_msgs::msg::Bool::SharedPtr msg)
{
  manual_control_enabled_ = !msg->data; // If msg->data is true (AUTO), manual_control_enabled_ becomes false. If msg->data is false (MANUAL), manual_control_enabled_ becomes true.
  RCLCPP_INFO(nh->get_logger(), "Control mode received: %s. Manual control enabled: %s",
              msg->data ? "AUTO" : "MANUAL", manual_control_enabled_ ? "true" : "false");
}

// void vel_cmd_callback(const geometry_msgs::msg::TwistStamped::SharedPtr msg)
// {
//     vx = msg->twist.linear.x;
//     vyaw = msg->twist.angular.z;
//     new_cmd = true;
// }

void joystickHandler(const sensor_msgs::msg::Joy::ConstSharedPtr joy)
{
//   joyTime = nh->now().seconds();
  joySpeedRaw = joy->axes[4];
  joySpeed = joySpeedRaw;
  joySpeedLateral = joy->axes[3] * maxSpeedLateral;

  joySpeedYaw = joy->axes[0] * maxSpeedYaw;

  if (joySpeed > 1.0) joySpeed = 1.0;
  if (joySpeed < -1.0) joySpeed = -1.0;
  if (joySpeedLateral > maxSpeedLateral) joySpeedLateral = maxSpeedLateral;
  if (joySpeedLateral < -maxSpeedLateral) joySpeedLateral = -maxSpeedLateral;
  
  if (joy->axes[4] == 0) joySpeed = 0;

  if (joy->axes[5] > -0.1) {
    checkObstacle = true;
  } else {
    checkObstacle = false;
  }

  new_cmd = true;
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);                             // Initialize rclcpp
    rclcpp::TimerBase::SharedPtr timer_;                  // Create a timer callback object to send cmd in time intervals
    nh = rclcpp::Node::make_shared("vel_cmd_repub"); // Create a ROS2 node and make share with low_level_cmd_sender class

    // state_suber = nh->create_subscription<unitree_go::msg::SportModeState>("sportmodestate", 10);
    
    // vel_cmd_suber = nh->create_subscription<geometry_msgs::msg::TwistStamped>("/cmd_vel", 10, vel_cmd_callback);

    joy_suber = nh->create_subscription<sensor_msgs::msg::Joy>("/joy", 5, joystickHandler);
    req_puber = nh->create_publisher<unitree_api::msg::Request>("/api/sport/request", 10);
    control_mode_suber = nh->create_subscription<std_msgs::msg::Bool>("/control_mode", 10, controlModeCallback); // Initialize control_mode subscriber

    rclcpp::Rate rate(100); // Set the frequency of the timer callback
    bool status = rclcpp::ok();
    while (status){
        rclcpp::spin_some(nh);
        vx = joySpeed;
        vyaw = joySpeedYaw;
        vy = joySpeedLateral;
        std::cout << "vx: " << vx << ",vy: " << vy << ", vyaw:" << vyaw << std::endl;
        
        if (manual_control_enabled_) {
            sport_req.Move(req, vx, vy, vyaw);
            req_puber->publish(req);
        } else {
            // If manual control is disabled, send a stop command
            sport_req.Move(req, 0, 0, 0);
            req_puber->publish(req);
        }
        
        //if (new_cmd)
        //{
        //    sport_req.Move(req, vx, 0, vyaw);
        //    req_puber->publish(req);
        //    new_cmd = false;
        //    count_down = 5;
        //}
        //else
        //{
        //    if (count_down > 0){
        //        count_down--;
        //        sport_req.Move(req, vx, 0, vyaw);
        //        req_puber->publish(req);
        //    }
        //    else{
        //        sport_req.Move(req, 0, 0, 0);
        //        req_puber->publish(req);
        //    }
        // }
        rate.sleep();
        status = rclcpp::ok();
    }

    rclcpp::shutdown();
    return 0;
}
