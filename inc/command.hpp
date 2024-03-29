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
#include "custom_environ.hpp"
#include "builtins.hpp"


class command {
public:
    std::vector<std::string> args;

    explicit command(std::vector<std::string> args_, std::vector<std::string> &desc, custom_environ &env);

    ~command();

    int execute_command(builtins_map &builtins, const int &close_fd = -1);

    void set_stdin(int &desc);

    void set_stdout(int &desc);

    void set_stderr(int &desc);

private:
    size_t background;
    size_t stdin_ = STDIN_FILENO, stdout_ = STDOUT_FILENO, stderr_ = STDERR_FILENO;
    custom_environ &environ;
};


#endif //BASHELLO_COMMAND_HPP
