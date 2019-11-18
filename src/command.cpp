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

command::command(std::vector<std::string> args, std::vector<std::string> &desc, custom_environ &env, size_t background)
        : args(std::move(args)),
          environ(env), background(background) {
    if (background) {
        background = open("/dev/null", O_WRONLY);
        stdout_ = background, stderr_ = background;
    }
    if (!desc[0].empty())
        // TODO: pomylka maye buty yaksho nema faila a ne create
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
    if (background) close(background);
}


int command::execute_command(builtins_map &builtins, const int &close_fd) {
    arguments_container arguments;
    int status;
    if (builtins.find(args[0]) != builtins.end()) {
        int stdout_copy = dup(STDOUT_FILENO);
        dup2(stdout_, STDOUT_FILENO);
        status = builtins[args[0]](args);
        dup2(stdout_copy, STDOUT_FILENO);
        if (status)
            std::cerr << ERR_COLOR << strerror(status) << RESET << '\n';
        return status;
    }
    arguments.buf = new const char *[args.size() + 1];   // extra room for sentinel
    for (int j = 0; j < args.size(); ++j) {     // copy args
        arguments.buf[j] = args[j].c_str();
    }
    arguments.buf[args.size()] = nullptr;  // end of arguments sentinel is NULL

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        if (stdin_ != STDIN_FILENO) dup2(stdin_, STDIN_FILENO);
        if (stdout_ != STDOUT_FILENO) dup2(stdout_, STDOUT_FILENO);
        if (stderr_ != STDERR_FILENO) dup2(stderr_, STDERR_FILENO);
        if (close_fd != -1) close(close_fd);
        int ret = execvpe(args[0].c_str(), (char **) arguments.buf, environ.c_arr());

        if (ret == -1 && errno == ENOENT)  // no such file or directory
            std::cerr << ERR_COLOR << args[0] << ": command not found" << RESET << '\n';

        _exit(0);
    } else if (background) return 0;
    else (void) waitpid(pid, &status, 0);
    return status;
}

void command::set_stdin(int &desc) {
    if (stdin_ != STDIN_FILENO) close(stdin_);
    stdin_ = desc;
}

void command::set_stdout(int &desc) {
    if (stdout_ != STDOUT_FILENO) close(stdout_);
    stdout_ = desc;
}

void command::set_stderr(int &desc) {
    if (stderr_ != STDERR_FILENO) close(stderr_);
    stderr_ = desc;
}