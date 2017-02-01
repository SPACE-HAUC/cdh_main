// Copyright 2017 UMass Lowell Command and Data Handling Team
/*!
 * @file
 * @brief Header of TopicMediator class
 *        A TopicMediator is a middle-man that handles communication between
 *        client code and a single ROS topic. It abstracts the details of how
 *        ROS works by providing simple functions to get and send data.
 *
 *        Note: TopicMediators both sub and pub to their topic, even if only
 *        one of those functions is necessary.
 */

#ifndef _TOPICMEDIATOR_H_
#define _TOPICMEDIATOR_H_

#include <string>

#include "ros/ros.h"

constexpr int ROS_MSG_BUFFER_MAX = 1000;

template<typename Data>
using CQ = boost::circular_buffer<Data>;


class TopicMediatorInterface {
 public:
  virtual void yield() = 0;
  virtual std::string topic() = 0;
  virtual ~TopicMediatorInterface() {}
};

template<typename Data>
class TopicMediator : public TopicMediatorInterface {
 private:
  std::string topic_name_;
  ros::Publisher pub_;
  ros::Subscriber sub_;

  CQ<Data> sub_data_Q_;
  CQ<Data> pub_data_Q_;
 public:
  TopicMediator() : sub_data_Q_(CQ<Data>(ROS_MSG_BUFFER_MAX)),
                    pub_data_Q_(CQ<Data>(ROS_MSG_BUFFER_MAX)) {}
  void publish_data(Data data);
  int get_data(Data *data);

  template<typename ConstDataPtr>
  void init(ros::NodeHandle *nh, std::string topic_name);
  std::string topic() { return topic_name_; }

  void yield();
};

#endif  /* _TOPICMEDIATOR_H_ */
