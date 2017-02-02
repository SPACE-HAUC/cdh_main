/*!
 * @file
 * @brief Mediator API idea 1
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

/*
 * CDH Mediator API with some degree of genericity.
 *
 * Pros:
 * - More generic
 *   - we only need to edit two switches and two list declarations
 *   - standardized way that every node functions
 *
 * Cons:
 * - May be complex for those unfamiliar with C++ inheritance and the
 *   C++11 standard
 * - Downcasting is spooky
 */

#include <string>
#include <map>
#include <list>

#define ROS_MSG_BUFFER_MAX 1000

template <typename Container, typename Lambda>
void for_each(Container container, Lambda lambda) {
  std::for_each(container.begin(), container.end(), lambda);
}


/***** We can reuse *****/

class Mediator {
 private:
  // necessary to keep around
  std::map<std::string, ros::Publisher>  topic_publishers_;
  std::map<std::string, ros::Subscriber> topic_subscribers_;
 protected:
  // CQ stands for circular queue
  std::map< std::string, CQ<CDHData> > topic_sub_queues_;
  std::map< std::string, CQ<CDHData> > topic_pub_queues_;
  NodeHandler node_handler_;

 public:
  virtual ~Mediator() {}
  virtual Mediator create() = 0;
  virtual void yield() = 0;
};


void Mediator::init(std::list<std::string> subTopics,
                    std::list<std::string> pubTopics, int waitMs) {
  node_handler_ = new NodeHandler(waitMs);
  for_each(subTopics, [](const std::string &topic) {
      topicSubQueues[topic] = new CQ<CDHData>();
      topic_publishers_[topic] = \
        node_handler_.listen(topic, ROS_MSG_BUFFER_MAX,
                             [](const CDHData &data) {
                               topic_sub_queues_[topic].enqueue(data);
                             });
    });
  for_each(pubTopics, [](const std::string &topic) {
      topic_pub_queues_[topic] = new CQ<CDHData>();
      topic_subscribers_[topic] = node_handler_.advertise(topic);
    });
}



/***** we must manually change the following for each team *****/

// must create these
class TeamInData : public CDHData;
class TeamOutData : public CDHData;

class ExampleMediator : public Mediator {
 private:
  ExampleMediator() {}

 public:
  int get_data(TeamInData *o);
  void publish_date(TeamOutData data);
  void init(int waitMs);
  ExampleMediator create(int waitMs = 50);
  void yield();
}

// must change whole function
void ExampleMediator::publish_data(TeamOutData data) {
  auto ACSData = data.x;
  auto PowerData = data.y;
  topic_pub_queues_["ACS"].enqueue(ACSData);
  topic_pub_queues_["power"].enqueue(PowerData);
}

// need to customize the switch cases
void ExampleMediator::yield() {
  for_each(topic_pub_queues_, [](Pair<std::string, CQ<CDHData>> &topicQueue) {
      CQ<CDHData> dataQ = topicQueue.second;
      std::string topicName = topicQueue.first;
      switch (topicName) {
      case "Logging":
        LogData* dataItem = std::dynamic_cast<LogData*>(&(dataQ.dequeue()));
        if (dataItem == NULL) {
          // wrong type of data in the queue
          return;
        }
        topic_subscribers_[topicName].publish(*dataItem);
      // ... for every pub topic
      // Alternatively, could customize publishing more by picking and
      // choosing data from the queues like in the manual example
      }
    });
  ros::spinOnce();
}

// need to customize the topic name lists
Mediator ExampleMediator::create(int waitMs = 50) {
  Mediator m = ExampleMediator();
  m.init(new std::list<std::string>{"ACS", "power"},
         new std::list<std::string>{"Logging"}, waitMs);
  return m;
}

// need to customize the switch cases
int ExampleMediator::get_data(TeamInData *o) {
  for_each(topic_sub_queues_, [](Pair<std::string, CQ<CDHData>> &topicQueue) {
      CQ<CDHData> dataQ = topicQueue.second;
      switch (topicQueue.first) {
      case "ACS":
        // ACSData defined in external header based on request forms
        ACSData* dataItem = std::dynamic_cast<ACSData*>(&(dataQ.dequeue()));
        if (dataItem == NULL || other error) {
          return -1;
        }
        o -> acs_x = dataItem.x;

      case "power":
        // PowerData defined in external header based on request forms
        PowerData *dataItem = std::dynamic_cast<PowerData*>(&(dataQ.dequeue()));
        if (dataItem == NULL || other error) {
          return -1;
        }
        o -> power_this = dataItem.this;
        o -> power_that = dataItem.that;

      // ... for every sub topic
      }
    });
}
