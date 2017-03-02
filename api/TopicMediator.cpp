/*!
 * @file
 * @brief Implementation of TopicMediator class
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

#include "TopicMediator.h"

#include <string>
#include <algorithm>

template<typename Data>
void TopicMediator<Data>::publish_data(Data data) {
  pub_data_Q_.push_back(data);
}

template<typename Data>
int TopicMediator<Data>::get_data(Data *data) {
  Data d = sub_data_Q_.front();
  sub_data_Q_.pop_front();
  std::copy(&d, &d + sizeof(d), data);
  return 1;
}

/*
  Note the extra template paramater ConstDataPtr: this should be ROS's
  msg_type::ConstPtr type so that the callback matches ROS's expected callback
  signature
*/
template<typename Data>
template<typename ConstDataPtr>
void TopicMediator<Data>::init(ros::NodeHandle *nh,
                               std::string topic_name) {
  pub_ = nh -> advertise<Data>(topic_name, ROS_MSG_BUFFER_MAX);
  sub_ = nh -> subscribe<Data>(topic_name, ROS_MSG_BUFFER_MAX,
                               [this](const ConstDataPtr &data){
                                 sub_data_Q_.push_back(*data);
                              });
}

template<typename Data>
void TopicMediator<Data>::yield() {
  while (pub_data_Q_.size() != 0) {
    pub_.publish(pub_data_Q_.front());
    pub_data_Q_.pop_front();
  }
}
