/*!
 * @file
 * @brief CircluarQueue class definition.
 */

// Copyright 2017 UMass Lowell Command and Data Handling Team

#ifndef API_CIRCULARQUEUE_H_
#define API_CIRCULARQUEUE_H_

template<typename Data>
class CircularQueue {
 public:
  /*!
   * Create a CircularQueue with capacity capacity
   */
  explicit CircularQueue(unsigned const int &capacity);

  ~CircularQueue() { delete[] queue_; }

  int size() { return num_items_; }

  bool isEmpty() { return (num_items_ == 0); }

  bool isFull() { return (num_items_ == capacity_); }

  /*!
   * Add an element to the front of the buffer
   */
  void enqueue(const Data &x);

  /*!
   * Remove an element from the end of the buffer
   * Throws std::runtime_error if queue is empty
   */
  Data dequeue();

  /*
   * What's at the end of the buffer?
   * Throws std::runtime_error if queue is empty
   */
  Data peek();

  void empty() { last_ = -1, first_ = num_items_ = 0; }

 private:
  Data *queue_;
  int first_, last_, num_items_, capacity_;
};  // end class CircularQueue

#endif  // API_CIRCULARQUEUE_H_
