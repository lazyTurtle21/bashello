#ifndef BASHELLO_PARSE_UTILS_HPP
#define BASHELLO_PARSE_UTILS_HPP

#include <cstdlib>
#include <iostream>
#include <vector>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include "wildcards.hpp"
#include "custom_environ.hpp"
#include "command.hpp"

void replace_vars(std::string& command);
std::vector<std::string> get_script_vars(std::string& command);
void split(std::string &source, std::vector<std::string> &destination);

void replace_wildcard(std::vector<std::string>& command);

int parse_variables(std::vector<std::string>& command, custom_environ &environ_);
std::string remove_leading_ending_spaces(const std::string& str);
std::pair<std::string, std::vector<std::string>> get_files_for_redirect(std::string cmd);
#endif //BASHELLO_PARSE_UTILS_HPP
