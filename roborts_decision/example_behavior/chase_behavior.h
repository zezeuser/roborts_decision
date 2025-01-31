#ifndef ROBORTS_DECISION_CHASE_BEHAVIOR_H
#define ROBORTS_DECISION_CHASE_BEHAVIOR_H

#include "io/io.h"

#include "../blackboard/blackboard.h"
#include "../executor/chassis_executor.h"
#include "../behavior_tree/behavior_state.h"
#include "../proto/decision.pb.h"

#include "line_iterator.h"

namespace roborts_decision {
class ChaseBehavior {
 public:
  ChaseBehavior(ChassisExecutor* &chassis_executor,
                Blackboard* &blackboard,
                const std::string & proto_file_path) : chassis_executor_(chassis_executor),
                                                       blackboard_(blackboard) {


    chase_goal_.header.frame_id = "map";
    chase_goal_.pose.orientation.x = 0;
    chase_goal_.pose.orientation.y = 0;
    chase_goal_.pose.orientation.z = 0;
    chase_goal_.pose.orientation.w = 1;

    chase_goal_.pose.position.x = 0;
    chase_goal_.pose.position.y = 0;
    chase_goal_.pose.position.z = 0;

    chase_buffer_.resize(2);
    chase_count_ = 0;
    chase_count_delay_ = 200;

    cancel_goal_ = true;
  }

  void Run() {

    auto executor_state = Update();
    auto robot_map_pose = blackboard_->GetRobotMapPose();
    auto enemy_pose = blackboard_->GetEnemy();
    blackboard_->SetMyToward(enemy_pose);
    auto dx = enemy_pose.pose.position.x - robot_map_pose.pose.position.x;
    auto dy = enemy_pose.pose.position.y - robot_map_pose.pose.position.y;
    auto yaw = std::atan2(dy, dx);
    geometry_msgs::PoseStamped toEnemy;
    toEnemy.pose.orientation = tf::createQuaternionMsgFromYaw(yaw);
    
    if (executor_state != BehaviorState::RUNNING 
        || (blackboard_->GetAngle(robot_map_pose, toEnemy) >= blackboard_->threshold.near_angle)) {
    //   auto robot_map_pose = blackboard_->GetRobotMapPose();
    //   auto enemy_pose = blackboard_->GetEnemy();

    //   auto dx = enemy_pose.pose.position.x - robot_map_pose.pose.position.x;
    //   auto dy = enemy_pose.pose.position.y - robot_map_pose.pose.position.y;
    //   auto yaw = std::atan2(dy, dx);
    //   geometry_msgs::PoseStamped toEnemy;
    //   toEnemy.pose.orientation = tf::createQuaternionMsgFromYaw(yaw);
      
      // 到一定距离则取消跟踪
      if ( (std::sqrt(std::pow(dx, 2) + std::pow(dy, 2)) >= 1.0) 
             && (std::sqrt(std::pow(dx, 2) + std::pow(dy, 2)) <= 2.0)
             && (blackboard_->GetAngle(robot_map_pose, toEnemy) <= blackboard_->threshold.near_angle) ) {
        if (cancel_goal_) {
          chassis_executor_->Cancel();
          blackboard_->info.is_chase = false;
          cancel_goal_ = false;
        }
        return;
      } 
      // 跟踪
      else {
        bool find_goal = false;
        geometry_msgs::PoseStamped reduce_goal;
        // reduce_goal.pose.orientation = robot_map_pose.pose.orientation;
        reduce_goal.header.frame_id = "map";
        reduce_goal.header.stamp = ros::Time::now();
        reduce_goal.pose.position.x = enemy_pose.pose.position.x - 1.0 * cos(yaw);
        reduce_goal.pose.position.y = enemy_pose.pose.position.y - 1.0 * sin(yaw);
        reduce_goal.pose.orientation = toEnemy.pose.orientation;
        chase_buffer_.push_back(reduce_goal);
        UpdateChaseBuffer(enemy_pose);
        for(auto iterator_goal = chase_buffer_.begin(); iterator_goal != chase_buffer_.end(); ++iterator_goal){
            geometry_msgs::PoseStamped enemy_goal;
            enemy_goal.pose.position.x = iterator_goal->pose.position.x;
            enemy_goal.pose.position.y = iterator_goal->pose.position.y;
            enemy_goal.pose.position.z =  0;
            unsigned int goal_cell_x, goal_cell_y;

            // if necessary add mutex lock
            //blackboard_->GetCostMap2D()->GetMutex()->lock();
            auto get_enemy_cell = blackboard_->GetCostMap2D()->World2Map(enemy_goal.pose.position.x,
                                                                    enemy_goal.pose.position.y,
                                                                    goal_cell_x,
                                                                    goal_cell_y);
            //blackboard_->GetCostMap2D()->GetMutex()->unlock();
            //faile
            if (!get_enemy_cell) {
            return;
            }

            // chase line iterator
            if ( (blackboard_->GetCostMap2D()->GetCost(goal_cell_x, goal_cell_y) >= 253)
            || (blackboard_->IsBombAllyGoal(enemy_goal)) ) {
            auto robot_x = robot_map_pose.pose.position.x;
            auto robot_y = robot_map_pose.pose.position.y;
            unsigned int robot_cell_x, robot_cell_y;
            double goal_x, goal_y;
            blackboard_->GetCostMap2D()->World2Map(robot_x,
                                                    robot_y,
                                                    robot_cell_x,
                                                    robot_cell_y);

            for(FastLineIterator line( goal_cell_x, goal_cell_y, robot_cell_x, robot_cell_y); line.IsValid(); line.Advance()) {

                auto point_cost = blackboard_->GetCostMap2D()->GetCost((unsigned int) (line.GetX()), (unsigned int) (line.GetY())); //current point's cost
            
                blackboard_->GetCostMap2D()->Map2World((unsigned int) (line.GetX()),
                                                        (unsigned int) (line.GetY()),
                                                        goal_x,
                                                        goal_y);
           
                if(point_cost >= 253 
                || blackboard_->IsBombAllyGoal(goal_x, goal_y)
                || blackboard_->GetEulerDistance(goal_x, goal_y, blackboard_->info.opp_reload.pose.position.x, blackboard_->info.opp_reload.pose.position.y) <= 1.0
                || blackboard_->GetEulerDistance(goal_x, goal_y, blackboard_->info.opp_shield.pose.position.x, blackboard_->info.opp_shield.pose.position.y) <= 1.0){
                continue;
                } else {
                    find_goal = true;
                    reduce_goal.pose.position.x = goal_x;
                    reduce_goal.pose.position.y = goal_y;
                    break;
                }
            }
          // 线段历遍完后则判断是否找到合适的点
            if (find_goal){
                cancel_goal_ = true;
                dx = enemy_pose.pose.position.x - reduce_goal.pose.position.x;
                dy = enemy_pose.pose.position.y - reduce_goal.pose.position.y;
                yaw = std::atan2(dy, dx);
                reduce_goal.pose.orientation = tf::createQuaternionMsgFromYaw(yaw);
                chassis_executor_->Execute(reduce_goal);
                blackboard_->SetMyGoal(reduce_goal);
                blackboard_->info.is_chase = true;
                ROS_INFO(" FIND GOAL !!!!!!!!  Yaw: %f" ,yaw);
                break;
            }
        } 
        else{
            cancel_goal_ = true;
            find_goal = true;
            chassis_executor_->Execute(reduce_goal);
            blackboard_->SetMyGoal(reduce_goal);
            blackboard_->info.is_chase = true;
            if(!blackboard_->info.has_my_enemy){
                chase_count_ ++ ;
            }
            if(chase_count_ >= chase_count_delay_){
                blackboard_->info.is_chase = false;
                chase_count_ = 0;
                chassis_executor_->Cancel();
                cancel_goal_ = true;
            }
            ROS_INFO(" go the enemy goal ");
            break;
        }
       } // 敌人周围四个点
       if(!find_goal) {
            blackboard_->info.is_chase = false;
            if (cancel_goal_) {
            chassis_executor_->Cancel();
            cancel_goal_ = false;
            }
            else {
                reduce_goal.pose.position.x = robot_map_pose.pose.position.x;
                reduce_goal.pose.position.y = robot_map_pose.pose.position.y;
                float ya = std::atan2(enemy_pose.pose.position.y - reduce_goal.pose.position.y, 
                                    enemy_pose.pose.position.x - reduce_goal.pose.position.x);
                reduce_goal.pose.orientation = tf::createQuaternionMsgFromYaw(ya);
                chassis_executor_->Execute(reduce_goal);
                blackboard_->SetMyGoal(reduce_goal);
                ROS_INFO(" CAN'T FIND GOAL and ambush the enemy!!!!!!!! yaw: %f " , ya);
            }
            ROS_INFO(" CAN'T FIND GOAL !!!!!!!!  " );
            return;
        }
       chase_buffer_.clear();
      }
    }
  }

  void UpdateChaseBuffer(geometry_msgs::PoseStamped enemy_pose){
    geometry_msgs::PoseStamped temp_goal;
    float dist = blackboard_->threshold.near_dist;
    temp_goal.header.frame_id = "map";
    temp_goal.header.stamp = ros::Time::now();
    temp_goal.pose.position.x = (enemy_pose.pose.position.x - dist) > 0 ? (enemy_pose.pose.position.x - dist) : enemy_pose.pose.position.x;
    temp_goal.pose.position.y = (enemy_pose.pose.position.y - dist) > 0 ? (enemy_pose.pose.position.y - dist) : enemy_pose.pose.position.y;
    chase_buffer_.push_back(temp_goal);
    temp_goal.pose.position.x = (enemy_pose.pose.position.x + dist) < 7.8 ? (enemy_pose.pose.position.x + dist) : enemy_pose.pose.position.x;
    temp_goal.pose.position.y = (enemy_pose.pose.position.y - dist) > 0 ? (enemy_pose.pose.position.y - dist) : enemy_pose.pose.position.y;
    chase_buffer_.push_back(temp_goal);
    temp_goal.pose.position.x = (enemy_pose.pose.position.x + dist) < 7.8 ? (enemy_pose.pose.position.x + dist) : enemy_pose.pose.position.x;
    temp_goal.pose.position.y = (enemy_pose.pose.position.y + dist) < 4.5 ? (enemy_pose.pose.position.y + dist) : enemy_pose.pose.position.y;
    chase_buffer_.push_back(temp_goal);
    temp_goal.pose.position.x = (enemy_pose.pose.position.x - dist) > 0 ? (enemy_pose.pose.position.x - dist) : enemy_pose.pose.position.x;
    temp_goal.pose.position.y = (enemy_pose.pose.position.y + dist) < 4.5 ? (enemy_pose.pose.position.y + dist) : enemy_pose.pose.position.y;
    chase_buffer_.push_back(temp_goal);
  }

  void Cancel() {
    chassis_executor_->Cancel();
  }

  BehaviorState Update() {
    return chassis_executor_->Update();
  }

  void SetGoal(geometry_msgs::PoseStamped chase_goal) {
    chase_goal_ = chase_goal;
  }

  ~ChaseBehavior() = default;

 private:
  //! executor
  ChassisExecutor* const chassis_executor_;

  //! perception information
  Blackboard* const blackboard_;

  //! chase goal
  geometry_msgs::PoseStamped chase_goal_;

  //! chase buffer
  std::vector<geometry_msgs::PoseStamped> chase_buffer_;

  unsigned int chase_count_;
  unsigned int chase_count_delay_;
  //! cancel flag
  bool cancel_goal_;
};
}

#endif //ROBORTS_DECISION_CHASE_BEHAVIOR_H