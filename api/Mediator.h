/*!
 * @file
 * @brief Header for Mediator class
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

#ifndef _MEDIATOR_H_
#define _MEDIATOR_H_

#include "TopicMediator.h"

#include <list>
#include <functional>
#include <string>

template<typename TeamDataIn, typename TeamDataOut>
class Mediator {
 protected:
  std::list<TopicMediatorInterface *> topic_mediators_;
  ros::NodeHandle node_handle_;

 public:
  explicit Mediator(std::function<void()> yield_work) {
    other_yield_work = yield_work;
  }
  ~Mediator();

  // Returns ROS status: is ROS still ok?
  bool yield();

  virtual void publish_data(TeamDataOut out_data) = 0;
  virtual int get_data(TeamDataIn *in_data) = 0;
  std::function<void()> other_yield_work;

  /*
    The real work of the mediator happens here thanks to SFINAE and variadic
    templates.
    The problem that the Mediator needs to solve is twofold:
    1. The Mediator needs to manage a variable number of TopicMediators
    2. Every TopicMediator manages a different data type

    Thus, ideally, we would like a solution that programmatically generates a
    Mediator with TopicMediators based on a given list of topics. If C++ had a
    macro system this would be trivial. Instead, we have to do it with
    templates. The basic idea is that we template paramaterize the init function
    to populate the list of TopicMediators using the a list of template
    paramaters that specifies how many TopicMediators we need and types of those
    TopicMediators.

    The reason why TWO template paramaters must be provided for each
    TopicMediator instead of just the one for the data type carried by the topic
    is a quirk of roscpp: the message callback function that ROS expects must
    accept a paramater of the special Boost type ConstPtr, which is a subtype of
    each ROS message type. Why this is so is anybody's guess. For example, the
    callback for a topic carrying std_msgs::Int32 must accept an
    std_msgs::Int32::ConstPtr, which is a distinct type from Int32 so it can't
    be deduced from Int32 automatically (even though it's the same for every
    type).

    So init() is a recursive function that creates a topic mediator for the
    first datatype in the template paramater list and then invokes itself again
    for the remaining parameters. By taking one type paramater pair off of the
    list at each iteration it eventually reaches the base case where K == 1 and
    there is only one pair of types left.

    The integer template parameter K is present to tell C++ when the base case
    has been reached. Otherwise, C++'s template system is not smart enough to
    deduce that only one pair means invoking the base case.

    Note also that the list of topic names should be the same length as the list
    of TopicMediator data type pairs (its length should be >= K), since one name
    is popped off for each TopicMediator; and the names should correspond
    positionally with the TopicMediator types in the template paramater list.

    The base case:
  */
  template<int k, typename Data, typename ConstDataPtr>
  typename std::enable_if<k == 1, void>::type
  init(std::list<std::string> topic_names) {
    auto topic_mediator = new TopicMediator<Data>();
    topic_mediator -> init<ConstDataPtr>(&node_handle_,
                                         topic_names.front());
    topic_mediators_.push_back(topic_mediator);
  }

  // General case:
  template<int k, typename Data, typename ConstDataPtr, typename... TopicDatas>
  typename std::enable_if<k != 1, void>::type
  init(std::list<std::string> topic_names) {
    init<1, Data, ConstDataPtr>(topic_names);
    topic_names.pop_front();
    return this -> init<k - 1, TopicDatas...>(topic_names);
  }
};

#endif  /* _MEDIATOR_H_ */
