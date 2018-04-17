#!/bin/bash

printf "Running tests"
printf "\n\n--------------------------------------------------\n"

printf ">>> Running test set 1 <<<\n\n"
./reboot_module_test --catch_system_errors=no

printf "\n\n--------------------------------------------------\n"
printf ">>> Running test set 2 <<<\n\n"
./babysit_test --catch_system_errors=no

printf "\n\n--------------------------------------------------\n"
printf ">>> Running test set 2 <<<\n\n"
./octopos_driver_test --catch_system_errors=no

printf "\n\n--------------------------------------------------\n"
printf "Done running tests."
