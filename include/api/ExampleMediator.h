/*!
 * @file
 * @brief Example of using Mediator API to make a node for team Example
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

#ifndef _EXAMPLEMEDIATOR_H_
#define _EXAMPLEMEDIATOR_H_

#include <functional>
#include <string>

#include "Mediator.h"

struct ExampleIn {
  int beam_steering_x;
  int temp;
};
struct ExampleOut {
  int x, y;
  std::string log;
};



class ExampleMediator : public Mediator<ExampleIn, ExampleOut> {
 private:
  explicit ExampleMediator(std::function<void()> yield_work)
    : Mediator<ExampleIn, ExampleOut>(yield_work) {}
 public:
  static ExampleMediator create();
  void publish_data(ExampleOut out_data);
  int get_data(ExampleIn *in_data);

  void init();
};

#endif  /* _EXAMPLEMEDIATOR_H_ */
