//
// Created by lazyturtle on 18.11.19.
//

#ifndef BASHELLO_EXECUTION_HPP
#define BASHELLO_EXECUTION_HPP

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include "command.hpp"
#include "builtins.hpp"
#include "custom_environ.hpp"
#include "buf_containers.hpp"
#include "parse_utils.hpp"

int pipe_exec(std::vector<command> &commands, custom_environ &environ_, builtins_map &builtins);

int prepare_and_execute(std::string string_args, custom_environ &environ_, builtins_map &builtins, int pipe_out=-1,
                        int pipe_in=-1);

int execute_file(std::string &filename, custom_environ &environ_, int &status, builtins_map &builtins);

std::string magic_transform(std::string input, int &status, custom_environ &environ_, builtins_map &builtins);



#endif //BASHELLO_EXECUTION_HPP
