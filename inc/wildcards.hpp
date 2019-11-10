#ifndef BASHELLO_WILDCARDS_HPP
#define BASHELLO_WILDCARDS_HPP

#include <boost/filesystem.hpp>
#include <vector>
#include<string>

std::vector<std::string> get_all_files(std::string& wildcard, std::string target_path);

#endif //BASHELLO_WILDCARDS_HPP
