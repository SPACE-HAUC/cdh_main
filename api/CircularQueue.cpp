/*!
 * @file
 * @brief CircluarQueue class implementation.
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team


#include "CircularQueue.h"

#include <stdexcept>

CircularQueue::CircularQueue(unsigned const int &capacity)
  : first_(0), last_(-1), num_items_(0), capacity_(capacity) {
  queue_ = new Data[capacity];
}

void CircularQueue::enqueue(const Data &x) {
  if (isFull()) {
    dequeue();
  }
  last_ = (last_ + 1)%capacity_;
  queue_[last_] = x;
  ++num_items_;
}

Data CircularQueue::dequeue() {
  if (isEmpty()) {  // TODO(llazarek): Need to determine what we want to do here
    throw std::runtime_error("dequeue: queue is empty");
  }
  --num_items_;
  Data result = queue_[first_];
  first_ = (first_ + 1)%capacity_;
  return result;
}

Data CircularQueue::peek() {
  if (isEmpty()) {  // TODO(llazarek): Need to determine what we want to do here
    throw std::runtime_error("peek: queue is empty");
  }
  return queue_[first_];
}
