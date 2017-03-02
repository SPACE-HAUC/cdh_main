/*!
 * @file
 * @brief Example of using Mediator API to make a node for team Example
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

#include <boost/circular_buffer.hpp>

#include <string>
#include <list>
#include <algorithm>
#include <functional>

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int32.h"

#include "TopicMediator.h"

// nice helper function
template <typename Container, typename Lambda>
void for_each(Container container, Lambda lambda) {
  std::for_each(container.begin(), container.end(), lambda);
}

typedef std_msgs::Int32 ACSData;
typedef std_msgs::Int32 PowerData;
typedef std_msgs::String LoggerData;
typedef std_msgs::Int32 BeamSteeringData;
typedef std_msgs::Int32 TempData;


// NOTE: not a part of ExampleMediator class!
void example_yield_work() {
  // other yield work for example mediator
}

ExampleMediator ExampleMediator::create() {
  return ExampleMediator(example_yield_work);
}

void ExampleMediator::init() {
  Mediator::init<5,  // number of topics pub/sub'ing to
                 // topic datatypes
                 // note that every ROS type must be paired with its ConstPtr
                 std_msgs::Int32, std_msgs::Int32::ConstPtr,    // Power
                 std_msgs::Int32, std_msgs::Int32::ConstPtr,    // ACS
                 std_msgs::String, std_msgs::String::ConstPtr,  // Logger
                 std_msgs::Int32, std_msgs::Int32::ConstPtr,    // BeamSteering
                 std_msgs::Int32, std_msgs::Int32::ConstPtr \   // Temp
                 >(std::list<std::string> \
                   {"Power",  // topic names corresponding positionally to types
                       "ACS",
                       "Logger",
                       "BeamSteering",
                       "Temp" });
}

void ExampleMediator::publish_data(ExampleOut out_data) {
  // split up the out_data into the parts that are going to each topic
  // and invoke the publish_data() of the appropriate TopicMediator

  // Casting safe because each TopicMediator can tell us through the interface
  // what topic they listen to (therefore what type they are)

  // extract the data we need and categorize it
  ACSData acs;
  acs.data = out_data.x;
  PowerData power;
  power.data = out_data.y;
  LoggerData logger;
  logger.data = out_data.log;

  for_each(topic_mediators_, [acs, power, logger](TopicMediatorInterface *tm){
      auto topic = tm -> topic();
      if (topic.compare("ACS") == 0) {
        auto acs_tm = dynamic_cast< TopicMediator<ACSData> *>(tm);
        acs_tm -> publish_data(acs);
      } else if (topic.compare("Power") == 0) {
        auto power_tm = dynamic_cast< TopicMediator<PowerData> *>(tm);
        power_tm -> publish_data(power);
      } else if (topic.compare("Logger") == 0) {
        auto logger_tm = dynamic_cast< TopicMediator<LoggerData> *>(tm);
        logger_tm -> publish_data(logger);
      }
      // ... for each pub topic
    });
}

int ExampleMediator::get_data(ExampleIn *in_data) {
  // iterate over each of the TopicMediators, pulling data from their queues

  // Casting safe because each TopicMediator can tell us through the interface
  // what topic they listen to (therefore what type they are)
  for_each(topic_mediators_, [in_data](TopicMediatorInterface * tm){
      auto topic = tm -> topic();
      if (topic.compare("BeamSteering") == 0) {
        auto bs_tm = dynamic_cast< TopicMediator<BeamSteeringData> *>(tm);
        BeamSteeringData bs_data;
        bs_tm -> get_data(&bs_data);
        in_data -> beam_steering_x = bs_data.data;
      } else if (topic.compare("Temp") == 0) {
        auto temp_tm = dynamic_cast< TopicMediator<TempData> *>(tm);
        TempData temp_data;
        temp_tm -> get_data(&temp_data);
        in_data -> temp = temp_data.data;
      }
      // ... for each sub topic
    });
  return 1;
}
