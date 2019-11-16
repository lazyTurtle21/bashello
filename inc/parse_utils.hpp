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

void replace_vars(std::vector<std::string>& command);

void split(std::string &source, std::vector<std::string> &destination);

void replace_wildcard(std::vector<std::string>& command, std::string& full_path,  std::string& dir, std::string& filename);

int parse_variables(std::vector<std::string>& command, custom_environ &environ_);
#endif //BASHELLO_PARSE_UTILS_HPP
