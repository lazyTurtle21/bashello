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
#include "../inc/wildcards.hpp"
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
    std::vector<std::string> command;
    command_line cmd_line;
    int status = 0;

    std::string pathenv = std::string(getenv("PATH")) + ":" + current_path;
    setenv("PATH", pathenv.c_str(), 1);
    custom_environ environ_(envp);

    builtins_map builtins = get_function(current_path, environ_, status);

    arguments_container arguments;


    while ((cmd_line.buf = readline((PATH_COLOR + current_path + RESET + DOLLAR + "$ " + RESET).c_str())) != nullptr) {
        if (strlen(cmd_line.buf) > 0) add_history(cmd_line.buf);

        string_args = cmd_line.buf;
        command.clear();

        split(string_args, command);
        replace_vars(command);
        status = parse_variables(command, environ_);
        if (status){
            std::cerr << ERR_COLOR << "error while creating a new variable: " << strerror(status) << RESET << '\n';
            continue;
        }
        if (command.empty()) continue;

        replace_wildcard(command, full_path, dir, filename);

        if (builtins.find(command[0]) != builtins.end()) {
            status = builtins[command[0]](command);
            if (status){
                std::cerr << ERR_COLOR << command[0] <<": " << strerror(status) << RESET << '\n';
            }

            continue;
        }
        arguments.buf = new const char *[command.size() + 1];   // extra room for sentinel

        for (int j = 0; j < command.size(); ++j) {     // copy args
            arguments.buf[j] = command[j].c_str();
        }

        arguments.buf[command.size()] = nullptr;  // end of arguments sentinel is NULL

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
            (void) waitpid(pid, &status, 0);
        }
    }

    return 0;
}