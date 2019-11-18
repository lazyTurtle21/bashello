#include <iostream>
#include <unistd.h>
#include <cstdlib>
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
#include "../inc/execution.hpp"

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


//    std::string input = "ls -l";
//    std::string output = magic_transform(input, status, environ_, builtins);
//    std::cout << output;

    if(argc == 2){
        std::string file{argv[1]};
        status = execute_file(file, environ_, status, builtins);
        return status;
    }

    while ((cmd_line.buf = readline((PATH_COLOR + current_path + RESET + DOLLAR + "$ " + RESET).c_str())) != nullptr) {
        if (strlen(cmd_line.buf) > 0) add_history(cmd_line.buf);

        string_args = cmd_line.buf;

        std::vector<std::string> command_;
        status = prepare_and_execute(string_args, environ_, builtins);
    }

    return 0;
}