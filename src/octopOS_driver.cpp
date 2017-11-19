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
	modules[module] = Module(pid, current_key, time(0));
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

void kill_module(std::string path, ModuleInfo *modules) {
    Module &module = (*modules)[path];
    module.killed = true;
    kill(module.pid, SIGTERM);
}

bool module_needs_downgrade(ModuleInfo module) {
    // TODO(llazarek): Is this a sufficient heuristic?
    return (time(0) - module.launch_time) < RUNTIME_CUTOFF_DOWNGRADE_S;
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
				 [](const Module &module) {module.pid == pid});
    if (moduleIt == modules.end()) {
	return Optional::None();
    } else {
	return Optional::Just(moduleIt -> first);
    }
}

class ChildHandler {
    static void sigchld_handler(int sig)
    {
	pid_t p;
	int status;
	int x = 0;
	pthread_t tmp;

	while ((pid = waitpid(-1, &status, WNOHANG)) != -1) {
	    // handle all dead children in new threads
	    pthread_create(&tmp, NULL, handle_dead_module, &x);
	}
    }

    static void handle_dead_module(pid_t pid) {
	Optional<std::string> found = find_module_with(pid, modules);
	if (found.isEmpty()) {
	    std::cerr << "Notification of unregistered module death. "
		      << "Something has probably gone horribly wrong."
		      << std::endl;
	} else {
	    reboot_module(found.get(), &modules);
	}
    }

    static void register_child_death_handler(ModuleInfo *_modules) {
	modules = *_modules;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigchld_handler;

	sigaction(SIGCHLD, &sa, NULL);
    }

private:
    static ModuleInfo &modules;
};
