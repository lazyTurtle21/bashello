//
// Created by yulianna on 11.11.19.
//

#ifndef BASHELLO_COMMAND_HPP
#define BASHELLO_COMMAND_HPP

#include <vector>
#include <cstring>
#include <string>
#include <fcntl.h> // Типи i константи для fcntl() i open().
#include <unistd.h>

class command {
public:
    std::vector<std::string> args;

    explicit command(std::vector<std::string> args, size_t stdin_ = STDIN_FILENO, size_t stdout_ = STDOUT_FILENO,
                     size_t stderr_ = STDERR_FILENO);

    ~command();

private:
    size_t stdin_, stdout_, stderr_;
};


#endif //BASHELLO_COMMAND_HPP
