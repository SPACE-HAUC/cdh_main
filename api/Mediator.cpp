/*!
 * @file
 * @brief Implementation of Mediator class
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

#include "Mediator.h"

// nice helper function
template <typename Container, typename Lambda>
void for_each(Container container, Lambda lambda) {
  std::for_each(container.begin(), container.end(), lambda);
}

template<typename TeamDataIn, typename TeamDataOut>
Mediator<TeamDataIn, TeamDataOut>::~Mediator() {
  for_each(topic_mediators_,
           [](TopicMediatorInterface *mediator){
             delete mediator;
           });
}

template<typename TeamDataIn, typename TeamDataOut>
bool Mediator<TeamDataIn, TeamDataOut>::yield() {
  for_each(topic_mediators_,
           [](TopicMediatorInterface *mediator){
             mediator -> yield();
           });
  other_yield_work();
  ros::spinOnce();

  return ros::ok();
}
