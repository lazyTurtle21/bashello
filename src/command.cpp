//
// Created by yulianna on 11.11.19.
//

#include "../inc/command.hpp"
#include "../inc/buf_containers.hpp"
#include <sys/wait.h>
#include <utility>
#include <iostream>

#define ERR_COLOR "\e[31m"

#define RESET "\u001b[0m"

void execute(std::vector<std::string> &command, arguments_container &arguments, custom_environ &environ_, int &status);

command::command(std::vector<std::string> args, std::vector<std::string> &desc, custom_environ &env)
        : args(std::move(args)),
          environ(env) {
    if (!desc[0].empty())
        stdin_ = open(desc[0].c_str(), O_TRUNC | O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (!desc[1].empty())
        stdout_ = open(desc[1].c_str(), O_TRUNC | O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (!desc[2].empty())
        stderr_ = open(desc[2].c_str(), O_TRUNC | O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}

command::~command() {

    if (stdin_ != STDIN_FILENO) close(stdin_);
    if (stdout_ != STDOUT_FILENO) close(stdout_);
    if (stderr_ != STDERR_FILENO) close(stderr_);

}


int command::execute_command(builtins_map &builtins) {
    arguments_container arguments;
    int status;
    if (builtins.find(args[0]) != builtins.end()) {
        status = builtins[args[0]](args);
        if (status) {
            std::cerr << ERR_COLOR << args[0] << ": " << strerror(status) << RESET << '\n';
        }
        return status;
    }
    arguments.buf = new const char *[args.size() + 1];   // extra room for sentinel
    for (int j = 0; j < args.size(); ++j) {     // copy args
        arguments.buf[j] = args[j].c_str();
    }
    arguments.buf[args.size()] = nullptr;  // end of arguments sentinel is NULL
    execute(args, arguments, environ, status);
    return status;

}

void execute(std::vector<std::string> &command, arguments_container &arguments, custom_environ &environ_, int &status) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        int ret = execvpe(command[0].c_str(), (char **) arguments.buf, environ_.c_arr());
        if (ret == -1 && errno == ENOENT) {     // no such file or directory
            std::cerr << ERR_COLOR << command[0] << ": command not found" << RESET << '\n';
        }
        _exit(0);
    } else {
        // IFKA na waitpid
        (void) waitpid(pid, &status, 0);
    }
}
