/*!
 * @file
 * @brief Mediator API idea 1
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

/*
 * CDH Mediator API with completely custom implementation for each subsystem.
 *
 * Pros:
 * - Simple for everyone involved
 *
 * Cons:
 * - Rewriting almost the entire implementation for every team
 *   - Could lead to each team having totally different node implementations,
 *     may be hard to maintain
 * - Gross..
 */

#include <string>

#define ROS_MSG_BUFFER_MAX 1000

template <typename Container, typename Lambda>
void for_each(Container container, Lambda lambda) {
  std::for_each(container.begin(), container.end(), lambda);
}

/***** we can reuse *****/
class MediatorInterface {
 public:
  virtual ~Mediator() {}
  virtual void init(int waitMs) = 0;
  virtual Mediator create(int waitMs = 50) = 0;
  virtual void yield() = 0;
};


/***** we must manually write for every team *****/

// this team listens to "Power" and "ACS" and "Other"
// and publishes to "BeamSteering" and "Logging"

class TeamInData;
class TeamOutData;

class ExampleMediator : public MediatorInterface {
 private:
  NodeHandler nodeHandler;

  ros::Publisher logging_publisher_;
  ros::Publisher beam_steering_publisher_;
  // ...

  ros::Subscriber power_subscriber_;
  ros::Subscriber ACS_subscriber_;
  ros::Subscriber other_subscriber_;
  // ...

  // CQ stands for circular queue
  CQ<int> pub_position_data_;
  CQ<std::string> pub_log_data_;
  CQ<double> pub_temp_data_;
  // ...


  CQ<PowerData> sub_power_data_;
  CQ<ACSData> sub_ACS_data_;
  CQ<OtherData> sub_other_data_;
  // ...

  ExampleMediator() {}

 public:
  int get_data(TeamInData *data);
  void publish_data(TeamOutData data);
  void init(int waitMs);
  ExampleMediator create(int waitMs = 50);
  void yield();
};


Mediator ExampleMediator::create(int waitMs = 50) {
  ExampleMediator m = ExampleMediator();
  m.init(waitMs);
  return m;
}

void ExampleMediator::init(int waitMs) {
  nodeHandler = new NodeHandler();
  nodeHandler.setLoopTime(waitMs);

  sub_power_data_ = new CQ<PowerData>();
  power_subscriber_ = nodeHandler.listen("Power", ROS_MSG_BUFFER_MAX,
                                         [](PowerData data) {
                                           sub_power_data_.enqueue(data);
                                         });
  sub_ACS_data_ = new CQ<ACSData>();
  ACS_subscriber_ = nodeHandler.listen("ACS", ROS_MSG_BUFFER_MAX,
                                       [](ACSData data) {
                                         sub_ACS_data_.enqueue(data);
                                       });
  sub_other_data_ = new CQ<OtherData>();
  other_subscriber_ = nodeHandler.listen("Other", ROS_MSG_BUFFER_MAX,
                                         [](OtherData data) {
                                           sub_other_data_.enqueue(data);
                                         });
  // ... for every subbed topic

  pub_position_data_ = new CQ<int>();
  pub_log_data_ = new CQ<std::string>();
  pub_temp_data_ = new CQ<double>();
  beam_steering_publisher_ = nodeHandler.advertise("BeamSteering");
  logging_publisher_ = nodeHandler.advertise("Logging");
  // ...  for every pub topic
}

void ExampleMediator::publish_data(TeamOutData data) {
  pub_log_data_.enqueue(format("log stuff %s", data.y));
  pub_position_data_.enqueue(data.x);
  pub_temp_data_.enqueue(data.y);
  // ... for every pub topic
}

int ExampleMediator::get_data(TeamInData *data) {
  int gotData = 0;
  auto power_data = sub_power_data_.dequeue();
  if (no error with power_data) {
    gotData++;
    data -> p = power_data;
  }
  auto ACS_data = sub_ACS_data_.dequeue();
  if (no error with ACS_data) {
    gotData++;
    data -> a = ACS_data;
  }
  auto other_data = sub_other_data_.dequeue();
  if (no error with other_data) {
    gotData++;
    data -> o = other_data;
  }
  // ... for every subbed topic

  return gotData;
}

void ExampleMediator::yield() {
  while (!pub_log_data_.is_empty()) {
    logging_publisher_.publish(pub_log_data_.dequeue());
  }

  while (!pub_position_data_.is_empty() && !pub_temp_data_.is_empty()) {
    BeamSteeringInfo info;
    info.p = pub_position_data_.dequeue();
    info.t = pub_temp_data_.dequeue();
    beam_steering_publisher_.publish(info);
  }

  ros::spinOnce();
}
