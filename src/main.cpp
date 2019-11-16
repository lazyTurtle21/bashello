#include <iostream>
#include "sys/types.h"
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include "../inc/command.hpp"
#include "../inc/builtins.hpp"
#include "../inc/custom_environ.hpp"
#include "../inc/buf_containers.hpp"
#include "../inc/parse_utils.hpp"

#define ERR_COLOR "\e[31m"
#define DOLLAR "\u001b[38;5;226m"
#define PATH_COLOR "\u001b[38;5;198m"
#define RESET "\u001b[0m"


int main(int argc, char *argv[], char *envp[]) {
    std::string string_args, full_path, dir, filename;
    std::string current_path = boost::filesystem::current_path().string();
    command_line cmd_line;
    int status = 0;

    std::string pathenv = std::string(getenv("PATH")) + ":" + current_path;
    setenv("PATH", pathenv.c_str(), 1);
    custom_environ environ_(envp);

    builtins_map builtins = get_function(current_path, environ_, status);


    while ((cmd_line.buf = readline((PATH_COLOR + current_path + RESET + DOLLAR + "$ " + RESET).c_str())) != nullptr) {
        if (strlen(cmd_line.buf) > 0) add_history(cmd_line.buf);

        string_args = cmd_line.buf;
        std::vector<std::string> command_;


        split(string_args, command_);
        replace_vars(command_);
        status = parse_variables(command_, environ_);
        if (status) {
            std::cerr << ERR_COLOR << "error while creating a new variable: " << strerror(status) << RESET << '\n';
            continue;
        }
        if (command_.empty()) continue;

        replace_wildcard(command_, full_path, dir, filename);

// TODO napisat' vector z descriptoramy imenamy
        std::vector<std::string> v{"f.txt", "", ""};
        command c(command_, v, environ_);
        status = c.execute_command(builtins);

    }

    return 0;
}