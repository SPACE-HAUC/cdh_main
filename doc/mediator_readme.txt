			   _________________

			    MEDIATOR README

			     Lukas Lazarek
			   _________________


Table of Contents
_________________

1 Summary
2 API
.. 2.1 Creating a Mediator
..... 2.1.1 Example
.. 2.2 Yielding
.. 2.3 Getting Data
..... 2.3.1 Example
.. 2.4 Publishing data
..... 2.4.1 Example





1 Summary
=========

  The Mediator is a library for simplifying interactions with the CDH
  communication infrastructure. It abstracts the details of asynchronous
  communication using ROS into a simple, unintrusive API.


2 API
=====

  CDH will provide each team with a specialized Mediator based on their
  communication needs.


2.1 Creating a Mediator
~~~~~~~~~~~~~~~~~~~~~~~

  A Mediator is created by simply invoking the `create' method of the
  specialized Mediator class for your team. This function returns a
  Mediator object through which you may communicate with CDH and other
  modules of the satellite.
  ,----
  | static SpecializedMediator SpecializedMediator::create();
  `----

  After creating a Mediator object, the object must be initialized by
  invoking the `init' method.
  ,----
  | void SpecializedMediator::init();
  `----
  Note: The Mediator object must not be used until after `init' has been
  invoked. Invoking any other Mediator methods before `init' is
  undefined.


2.1.1 Example
-------------

  ,----
  | #include "ExampleMediator.h"
  | 
  | int main(int argc, char *argv[]){
  |   ExampleMediator mediator = ExampleMediator::create();
  |   mediator.init(); // *required*
  |   
  |   // mediator automatically cleans up after itself when it destructs,
  |   // no need for special cleanup code
  |   return 0;
  | }
  `----


2.2 Yielding
~~~~~~~~~~~~

  The `yield' function yields control from to the mediator to allow it
  to handle its internal operations. The most important part of those
  operations is publishing the data that has been requested to be
  published and pulling any newly available data. The Mediator does not
  publish immediately when `publish_data' is invoked, only once `yield'
  is invoked. `publish_data' can be thought of as queueing up data to be
  published and `yield' can be thought of as emptying that queue (by
  actually publishing). Likewise, `yield' can be thought of as
  populating a queue of available data and `get_data' as popping an
  element off that queue. Thus, it is in your best interest to invoke
  yield() often. Depending upon the criticality of the timeliness of
  your communications, very often.
  ,----
  | void SpecializedMediator::yield();
  `----

  Please refer to the below examples to see how `yield' interacts with
  publishing and getting data.


2.3 Getting Data
~~~~~~~~~~~~~~~~

  The specialized mediator created by CDH for your team will be
  pre-setup to obtain and publish the information that your team
  specified in the Data Communications forms. Thus, getting the data
  that you requested on that form is as simple as invoking
  `SpecializedMediator::get_data' and providing a pointer to a `struct'
  (defined in the `SpecializedMediator' header) that contains fields
  corresponding to those requested on your teams Communications forms.
  The `struct' for input and output data are named `InData' and
  `OutData'.
  ,----
  | int get_data(InData *);
  `----

  `get_data' populates the `struct' at the given address and returns the
  number of data items filled.
   Return value  Meaning                                       
  -------------------------------------------------------------
   	   1  Successfully read data and populated `struct' 
   	   0  No data available, `struct' unmodified        


2.3.1 Example
-------------

  ,----
  | #include <iostream>
  | #include <chrono> // for sleep
  | #include <thread> // for sleep
  | #include "ExampleMediator.h"
  | 
  | /*
  |   ~~ Defined in ExampleMediator.h ~~
  |   struct InData {
  |     int x;
  |   };
  | */
  | 
  | int main(int argc, char *argv[]){
  |   ExampleMediator mediator = ExampleMediator::create();
  |   mediator.init(); // *required*
  | 
  |   InData message;
  |   while(true){
  |     mediator.yield(); // check for new data
  |     if (mediator.get_data(&message)) {
  |       std::cout << "x has been published: " << message.x << std::endl;
  |     } else {
  |       std::cout << "No data available..." << std::endl;
  |     }
  |     std::this_thread::sleep_for(std::chrono::seconds(1));
  |   }
  |   return 0;
  | }
  `----


2.4 Publishing data
~~~~~~~~~~~~~~~~~~~

  Publishing data is inversely analagous to getting data; simply invoke
  `publish_data' and provide a `OutData' object containing the data to
  publish.
  ,----
  | void SpecializedMediator::publish_data(OutData);
  `----


2.4.1 Example
-------------

  ,----
  | #include <iostream>
  | #include <chrono> // for sleep
  | #include <thread> // for sleep
  | #include "ExampleMediator.h"
  | 
  | /*
  |   ~~ Defined in ExampleMediator.h ~~
  |   struct OutData {
  |     string msg;
  |   };
  | */
  | 
  | int main(int argc, char *argv[]){
  |   ExampleMediator mediator = ExampleMediator::create();
  |   mediator.init(); // *required*
  | 
  |   OutData message;
  |   while(true){
  |     message.msg = "Hello world!";
  |     mediator.publish_data(message);
  |     mediator.yield(); // execute publish
  |     std::this_thread::sleep_for(std::chrono::seconds(1));
  |   }
  |   return 0;
  | }
  `----
