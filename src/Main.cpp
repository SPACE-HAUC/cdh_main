/*!
 * @file
 * @brief Program driver. Receives commands, and acts upon them.
 */

// Copyright 2016 UMass Lowell Command and Data Handling Team

void setup();
void loop();

int main(int argc, char const *argv[]) {
  setup();
  loop();
  return 0;
}

/*!
 * Initializes core components of program, and begins a networking thread with
 * ground station. 
 */
void setup() {
  // TODO(Team Req): Give functionality
}

/*!
 * Takes commands from ground station, queues them up, and runs them by 
 * command priority. 
 */
void loop() {
  while (true) {
    // TODO(Team Req): Give functionality
  }
}
