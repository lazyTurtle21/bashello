//
// Created by yulianna on 11.11.19.
//

#include "../inc/command.hpp"

#include <utility>

command::command(std::vector<std::string> args, size_t stdin_, size_t stdout_, size_t stderr_)
        : args(std::move(args)),
          stdin_(stdin_),
          stdout_(stdout_),
          stderr_(stderr_) {}

command::~command() {
// Ya sze don't know czy to good destructor (ne norm zakrywaye progu)
// TODO: norm destructor hocza hz czy treba ne default

//    close(stdin_);
//    close(stdout_);
//    close(stderr_);

}
