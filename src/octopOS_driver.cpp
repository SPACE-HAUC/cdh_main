// Copyright 2017 Space HAUC Command and Data Handling
// This file is part of Space HAUC which is released under AGPLv3.
// See file LICENSE.txt or go to <http://www.gnu.org/licenses/> for full
// license details.

/*!
 * @file
 *
 * @brief The octopOS driver program. Implements satellite process
 * initialization, management, and error handling.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <dirent.h>

#include <octopOS/octopos.h>
#include <octopOS/tentacle.h>

#include "../include/octopOS_driver.hpp"

bool accessible(FilePath file);
std::list<FilePath> modules_in(FilePath dir);
Optional< std::list<FilePath> > files_in(FilePath dir);


Optional<json> load(FilePath json_file) {
    Optional<json> out = Optional::None();
    if (accessible(json_file)) {
	json j;
	std::ifstream in(json_file);
	in >> j;
	out = Optional::Just(j);
    }
    return out;
}

// launches the given module in a new child process
pid_t launch(FilePath module, MemKey key) {
    pid_t pid;
    if (!(pid = fork())) { // child
	execl(module, std::to_string(key).c_str(), (char*)0);
    }
    return pid;
}

ModuleInfo launch_modules_in(FilePath dir, MemKey start_key) {
    ModuleInfo modules;
    long current_key = start_key;
    pid_t pid;
    for (FilePath module : modules_in(dir)) {
	pid_t pid = launch(module, current_key);
	modules[module] = Module(pid, current_key);
	current_key++;
    }
    return modules;
}

bool launch_listeners() {
    int x = 0;
    pthread_t tmp, sub_thread;

    if (pthread_create(&tmp, NULL, octopOS::listen_for_child, &x)) {
	return false;
    }

    if (pthread_create(&sub_thread, NULL,
		       subscriber_manager::wait_for_data, &x)) {
	return false;
    }

    return true;
}


// Currently just returns all the files in DIR.
//
// In the future we may need to do something more complex than just
// the files in the given directory - for example if a folder
// structure is necessary (for whatever reason).
std::list<FilePath> modules_in(FilePath dir) {
    auto files = files_in(dir);
    if (files.isEmpty()) {
	std::cerr << "Error: Unable to read module path from config: " << dir
		  << std::endl;
    }

    return files.getDefault(std::list<FilePath>());
}

Optional< std::list<FilePath> > files_in(FilePath dir) {
    DIR *dir;
    DIR *dir;
    struct dirent *ent;
    Optional< std::list<FilePath> > list = Optional::None();

    dir = opendir("./");
    if (dir != NULL){
	std::list<FilePath> files;
	while (epdf = readdir(dir)){
	    files.push(ent->d_name);
	}
	list = Optional::Just(files);
    }
    closedir(dir);
    return list;
}

// Courtesy of:
// https://stackoverflow.com/questions/12774207/fastest-way-to-check-if
// -a-file-exist-using-standard-c-c11-c
bool accessible(FilePath file) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
