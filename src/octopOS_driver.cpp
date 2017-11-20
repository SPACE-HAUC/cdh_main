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
#include <list>
#include <fstream>

#include <octopOS/octopos.h>
#include <octopOS/subscriber.h>
#include "../include/octopOS_driver.hpp"

bool accessible(FilePath file);
std::list<FilePath> modules_in(FilePath dir);
Optional< std::list<FilePath> > files_in(FilePath dir);


Optional<json> load(FilePath json_file) {
    Optional<json> out = Optional<json>::None();
    if (accessible(json_file)) {
	std::ifstream in(json_file);
	out = Optional<json>::Just(json::parse(in));
    }
    return out;
}

// launches the given module in a new child process
pid_t launch(FilePath module, MemKey key) {
    pid_t pid;
    if (!(pid = fork())) { // child
        execl(module.c_str(), std::to_string(key).c_str(), (char*)0);
    }
    return pid;
}

LaunchInfo launch_modules_in(FilePath dir, MemKey start_key) {
    ModuleInfo modules;
    MemKey current_key = start_key;
    pid_t pid;
    for (FilePath module : modules_in(dir)) {
	pid_t pid = launch(module, current_key);
	modules[module] = Module(pid, current_key, time(0));
	current_key++;
    }
    return std::make_pair(modules, current_key);
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

Optional< std::list<FilePath> > files_in(FilePath directory) {
    DIR *dir;
    struct dirent *ent;
    Optional< std::list<FilePath> > list =
      Optional< std::list<FilePath> >::None();

    dir = opendir(directory.c_str());
    if (dir != NULL){
	std::list<FilePath> files;
	while (ent = readdir(dir)){
	    files.push_front(ent->d_name);
	}
	list = Optional< std::list<FilePath> >::Just(files);
    }
    closedir(dir);
    return list;
}

// Courtesy of:
// https://stackoverflow.com/questions/12774207/fastest-way-to-check-if
// -a-file-exist-using-standard-c-c11-c
bool accessible(FilePath file) {
    struct stat buffer;
    return (stat (file.c_str(), &buffer) == 0);
}

void kill_module(std::string path, ModuleInfo *modules) {
    Module &module = (*modules)[path];
    module.killed = true;
    kill(module.pid, SIGTERM);
}

bool module_needs_downgrade(Module module) {
    // TODO(llazarek): Is this a sufficient heuristic?
    return (time(0) - module.launch_time) < RUNTIME_CUTOFF_DOWNGRADE_S;
}

void downgrade(FilePath module_name) {
    // TODO(llazarek): Should this be done here, or should the
    // procedure for downgrading be handled by the upgrade module? It
    // seems to make more sense in the upgrade module; here, we could
    // publish a request to downgrade the module.
}

void reboot_module(std::string path, ModuleInfo *modules) {
    Module &module = (*modules)[path];
    if (module.killed) {
	// Death was intentional, just reboot
	module.killed = false;
	module.pid = launch(path, module.msgkey);
	module.launch_time = time(0);
    } else {
	// Something else happened
	if (module_needs_downgrade(module)) {
	    downgrade(path);
	} else {
	    module.pid = launch(path, module.msgkey);
	    module.launch_time = time(0);
	}
    }
}

Optional<std::string> find_module_with(pid_t pid, ModuleInfo &modules) {
    auto moduleIt = std::find_if(modules.begin(), modules.end(),
				 [&](const std::pair<std::string, Module> el) {
				     return el.second.pid == pid;
				 });
    if (moduleIt == modules.end()) {
	return Optional<std::string>::None();
    } else {
	return Optional<std::string>::Just(moduleIt -> first);
    }
}
