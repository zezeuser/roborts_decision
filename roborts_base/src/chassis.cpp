/****************************************************************************
 *  Copyright (C) 2021 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#include "chassis.h"

namespace roborts_base{
Chassis::Chassis(std::shared_ptr<roborts_sdk::Handle> handle):
    Module(handle){
  SDK_Init();
  ROS_Init();
}
Chassis::~Chassis(){
  if(heartbeat_thread_.joinable()){
    heartbeat_thread_.join();
  }
}
void Chassis::SDK_Init(){

  verison_client_ = handle_->CreateClient<roborts_sdk::cmd_version_id,roborts_sdk::cmd_version_id>
      (UNIVERSAL_CMD_SET, CMD_REPORT_VERSION,
       MANIFOLD2_ADDRESS, CHASSIS_ADDRESS);
  roborts_sdk::cmd_version_id version_cmd;
  version_cmd.version_id=0;
  auto version = std::make_shared<roborts_sdk::cmd_version_id>(version_cmd);
  verison_client_->AsyncSendRequest(version,
                                    [](roborts_sdk::Client<roborts_sdk::cmd_version_id,
                                                           roborts_sdk::cmd_version_id>::SharedFuture future) {
                                      ROS_INFO("Chassis Firmware Version: %d.%d.%d.%d",
                                               int(future.get()->version_id>>24&0xFF),
                                               int(future.get()->version_id>>16&0xFF),
                                               int(future.get()->version_id>>8&0xFF),
                                               int(future.get()->version_id&0xFF));
                                    });

  handle_->CreateSubscriber<roborts_sdk::cmd_chassis_info>(CHASSIS_CMD_SET, CMD_PUSH_CHASSIS_INFO,
                                                           CHASSIS_ADDRESS, MANIFOLD2_ADDRESS,
                                                           std::bind(&Chassis::ChassisInfoCallback, this, std::placeholders::_1));
  handle_->CreateSubscriber<roborts_sdk::cmd_uwb_info>(COMPATIBLE_CMD_SET, CMD_PUSH_UWB_INFO,
                                                       CHASSIS_ADDRESS, MANIFOLD2_ADDRESS,
                                                       std::bind(&Chassis::UWBInfoCallback, this, std::placeholders::_1));

  chassis_speed_pub_ = handle_->CreatePublisher<roborts_sdk::cmd_chassis_speed>(CHASSIS_CMD_SET, CMD_SET_CHASSIS_SPEED,
                                                                                MANIFOLD2_ADDRESS, CHASSIS_ADDRESS);
  chassis_spd_acc_pub_ = handle_->CreatePublisher<roborts_sdk::cmd_chassis_spd_acc>(CHASSIS_CMD_SET, CMD_SET_CHASSIS_SPD_ACC,
                                                                                    MANIFOLD2_ADDRESS, CHASSIS_ADDRESS);

  heartbeat_pub_ = handle_->CreatePublisher<roborts_sdk::cmd_heartbeat>(UNIVERSAL_CMD_SET, CMD_HEARTBEAT,
                                                                        MANIFOLD2_ADDRESS, CHASSIS_ADDRESS);
  heartbeat_thread_ = std::thread([this]{
                                    roborts_sdk::cmd_heartbeat heartbeat;
                                    heartbeat.heartbeat=0;
                                    while(ros::ok()){
                                      heartbeat_pub_->Publish(heartbeat);
                                      std::this_thread::sleep_for(std::chrono::milliseconds(300));
                                    }
                                  }
  );
    ros_sub_gimbal =  ros_nh_.subscribe<roborts_msgs::GimbalInfo>("gimbal_info", 100, &Chassis::UpdateGimbalDataCallBack, this);
    ros_swing_mode = ros_nh_.subscribe<roborts_msgs::SwingMode>("swing_mode", 10, &Chassis::UpdateSwingModeCallBack ,this);
}
void Chassis::ROS_Init(){
  //ros publisher
  ros_odom_pub_ = ros_nh_.advertise<nav_msgs::Odometry>("odom", 30);
  ros_gimbal_odom_pub_ = ros_nh_.advertise<nav_msgs::Odometry>("gimbal_odom", 30);
  ros_uwb_pub_ = ros_nh_.advertise<geometry_msgs::PoseStamped>("uwb", 30);
  //ros subscriber
  ros_sub_cmd_chassis_vel_ = ros_nh_.subscribe("cmd_vel", 1, &Chassis::ChassisSpeedCtrlCallback, this);
  ros_sub_cmd_chassis_vel_acc_ = ros_nh_.subscribe("cmd_vel_acc", 1, &Chassis::ChassisSpeedAccCtrlCallback, this);
  

  //ros_message_init
  odom_.header.frame_id = "odom";
  odom_.child_frame_id = "base_link";
  odom_tf_.header.frame_id = "odom";
  odom_tf_.child_frame_id = "base_link";

  gimbal_odom_.header.frame_id = "odom";
  gimbal_odom_.child_frame_id = "gimbal_odom";
  gimbal_odom_tf_.header.frame_id = "odom";
  gimbal_odom_tf_.child_frame_id = "gimbal_odom";

  uwb_data_.header.frame_id = "uwb";
}
void Chassis::ChassisInfoCallback(const std::shared_ptr<roborts_sdk::cmd_chassis_info> chassis_info){

  ros::Time current_time = ros::Time::now();
  odom_.header.stamp = current_time;
  odom_.pose.pose.position.x = chassis_info->position_x_mm/1000.;
  odom_.pose.pose.position.y = chassis_info->position_y_mm/1000.;
  odom_.pose.pose.position.z = 0.0;
  geometry_msgs::Quaternion q = tf::createQuaternionMsgFromYaw(chassis_info->gyro_angle / 1800.0 * M_PI);
  odom_.pose.pose.orientation = q;
  odom_.twist.twist.linear.x = chassis_info->v_x_mm / 1000.0;
  odom_.twist.twist.linear.y = chassis_info->v_y_mm / 1000.0;
  odom_.twist.twist.angular.z = chassis_info->gyro_rate / 1800.0 * M_PI;
  ros_odom_pub_.publish(odom_);
  odom_tf_.header.stamp = current_time;
  odom_tf_.transform.translation.x = chassis_info->position_x_mm/1000.;
  odom_tf_.transform.translation.y = chassis_info->position_y_mm/1000.;
  odom_tf_.transform.translation.z = 0.0;
  odom_tf_.transform.rotation = q;
  tf_broadcaster_.sendTransform(odom_tf_);

  // gimbal odom  将底盘速度转换成云台坐标系的速度
  gimbal_odom_.header.stamp = current_time;
  gimbal_odom_.pose.pose.position.x = chassis_info->position_x_mm/1000.;
  gimbal_odom_.pose.pose.position.y = chassis_info->position_y_mm/1000.;
  gimbal_odom_.pose.pose.position.z = 0.0;
  geometry_msgs::Quaternion gimbal_q = tf::createQuaternionMsgFromYaw(gimbal_yaw_);
  gimbal_odom_.pose.pose.orientation = gimbal_q;
  gimbal_odom_.twist.twist.linear.x =  chassis_info->v_x_mm / 1000.0 * cos(relative_yaw_) + chassis_info->v_y_mm / 1000.0 * sin(relative_yaw_);
  gimbal_odom_.twist.twist.linear.y =  -chassis_info->v_x_mm / 1000.0 * sin(relative_yaw_) + chassis_info->v_y_mm / 1000.0 * cos(relative_yaw_);
  gimbal_odom_.twist.twist.angular.z = gimbal_yaw_rate_;
  ros_gimbal_odom_pub_.publish(gimbal_odom_);
  gimbal_odom_tf_.header.stamp = current_time;
  gimbal_odom_tf_.transform.translation.x = 0;
  gimbal_odom_tf_.transform.translation.y = 0;
  gimbal_odom_tf_.transform.translation.z = 0.0;
  gimbal_odom_tf_.transform.rotation = gimbal_q;
  tf_broadcaster_.sendTransform(gimbal_odom_tf_);
}

void Chassis::UWBInfoCallback(const std::shared_ptr<roborts_sdk::cmd_uwb_info> uwb_info){
  uwb_data_.header.stamp = ros::Time::now();
  uwb_data_.pose.position.x = ((double)uwb_info->x)/100.0;
  uwb_data_.pose.position.y = ((double)uwb_info->y)/100.0;
  uwb_data_.pose.position.z = 0;
  uwb_data_.pose.orientation = tf::createQuaternionMsgFromYaw(uwb_info->yaw/ 180.0 * M_PI);
  ros_uwb_pub_.publish(uwb_data_);

}

void Chassis::ChassisSpeedCtrlCallback(const geometry_msgs::Twist::ConstPtr &vel){
  roborts_sdk::cmd_chassis_speed chassis_speed;
  chassis_speed.vx = vel->linear.x*1000;
  chassis_speed.vy = vel->linear.y*1000;
  chassis_speed.vw = vel->angular.z * 1800.0 / M_PI;
  chassis_speed.rotate_x_offset = 0;
  chassis_speed.rotate_y_offset = 0;
  chassis_speed_pub_->Publish(chassis_speed);
}

void Chassis::ChassisSpeedAccCtrlCallback(const roborts_msgs::TwistAccel::ConstPtr &vel_acc){
  float degree_45 = 0.785;
  float swing_twist;
  // relative_yaw_ 云台与底盘的偏差角度 
  roborts_sdk::cmd_chassis_spd_acc chassis_spd_acc;
  chassis_spd_acc.vx = vel_acc->twist.linear.x*1000*cos(relative_yaw_) - vel_acc->twist.linear.y*1000*sin(relative_yaw_) ;
  chassis_spd_acc.vy = vel_acc->twist.linear.x*1000*sin(relative_yaw_) + vel_acc->twist.linear.y*1000*cos(relative_yaw_);
  if(swing_mode_){
    //  pitch_direction : True mean chaiss left rotating  , False mean chaiss right rotating
    yaw_direction_ = relative_yaw_ >= degree_45 ? true : yaw_direction_;
    yaw_direction_ = relative_yaw_ < -degree_45 ? false : yaw_direction_; 
    if(yaw_direction_){
        swing_twist = 1.5 * 1800.0 / M_PI;
    }else{
        swing_twist = -1.5 * 1800.0 / M_PI;
    }
    chassis_spd_acc.vw = vel_acc->twist.angular.z * 1800.0 / M_PI + swing_twist ;
  }
  else{
    chassis_spd_acc.vw = vel_acc->twist.angular.z * 1800.0 / M_PI;
  }
  chassis_spd_acc.ax = vel_acc->accel.linear.x*1000*cos(relative_yaw_) - vel_acc->accel.linear.y*1000*sin(relative_yaw_) ;
  chassis_spd_acc.ay = vel_acc->accel.linear.x*1000*sin(relative_yaw_) + vel_acc->accel.linear.y*1000*cos(relative_yaw_);
  chassis_spd_acc.wz = vel_acc->accel.angular.z * 1800.0 / M_PI;
  chassis_spd_acc.rotate_x_offset = 0;
  chassis_spd_acc.rotate_y_offset = 0;
  chassis_spd_acc_pub_->Publish(chassis_spd_acc);
}

void Chassis::UpdateGimbalDataCallBack(const roborts_msgs::GimbalInfo::ConstPtr &gimbal_info){
    gimbal_yaw_rate_ = gimbal_info->yaw_rate/ 1800.0 * M_PI;
    gimbal_yaw_ = gimbal_info->gyro_yaw/1800.0 * M_PI;
    relative_yaw_ = gimbal_info->ecd_yaw/1800.0 * M_PI;
}

void Chassis::UpdateSwingModeCallBack(const roborts_msgs::SwingMode::ConstPtr &swing_mode){
    swing_mode_ = swing_mode->swing_mode;
    if (!swing_mode->guidance)
    {
        float amplitude = 0.35;  // 0.35 = 20degree   amplityde 是扭腰限幅角度
        float swing_twist;
        roborts_sdk::cmd_chassis_spd_acc chassis_spd_acc;
        yaw_direction_ = relative_yaw_ >= amplitude ? true : yaw_direction_;
        yaw_direction_ = relative_yaw_ <= -amplitude ? false : yaw_direction_; 
        //  pitch_direction : True mean chaiss left rotating  , False mean chaiss right rotating
        if(yaw_direction_){
            swing_twist = 3.0 * 1800.0 / M_PI * cos(relative_yaw_) ;
        }else{
            swing_twist = -3.0 * 1800.0 / M_PI * cos(relative_yaw_) ;
        }
        chassis_spd_acc.vx = 0 ;
        chassis_spd_acc.vy = 0 ;
        chassis_spd_acc.vw = swing_twist ;
        chassis_spd_acc.ax = 0;
        chassis_spd_acc.ay = 0;
        chassis_spd_acc.wz = 0;
        chassis_spd_acc.rotate_x_offset = 0;
        chassis_spd_acc.rotate_y_offset = 0;
        chassis_spd_acc_pub_->Publish(chassis_spd_acc);
    }
    
}

}
