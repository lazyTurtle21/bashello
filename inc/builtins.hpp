#ifndef BASHELLO_BUILTINS_HPP
#define BASHELLO_BUILTINS_HPP

#include <string>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "custom_environ.hpp"
#include <map>
#include <getopt.h>

#define HELP_COLOR "\u001b[38;5;37m"
#define RESET "\u001b[0m"
#define ERR_COLOR "\e[31m"
#define DOLLAR "\u001b[38;5;226m"
#define PATH_COLOR "\u001b[38;5;198m"

typedef std::function<int(std::vector<std::string>&)> function;
typedef std::map<std::string, function> builtins_map;

int mcd(std::vector<std::string> &args, std::string &current_path);

int mexit(std::vector<std::string>& args);

int mecho(std::vector<std::string>& args);

int mexport(std::vector<std::string> &command, custom_environ &environ_);

int mpwd(std::vector<std::string> &args, const std::string &current_path);

int execute_file(std::string &filename, custom_environ &environ_, int &status, builtins_map& builtins);
int prepare_and_execute(std::string string_args, custom_environ &environ_, builtins_map &builtins);

builtins_map
get_function(std::string &current_path, custom_environ &environ_, int &status);


#endif //BASHELLO_BUILTINS_HPP
