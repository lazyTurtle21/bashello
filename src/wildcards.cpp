#include "../inc/wildcards.hpp"
#include <fnmatch.h>


std::vector<std::string> get_all_files(std::string& wildcard, std::string target_path) {
    std::vector<std::string> matching_files;
    if (!boost::filesystem::is_directory(target_path)) { return matching_files; }

    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for (boost::filesystem::directory_iterator i(target_path); i != end_itr; ++i) {
        if (!boost::filesystem::is_regular_file(i->status())) continue;
        if (fnmatch(wildcard.c_str(), i->path().filename().c_str(), 0)) continue;
        matching_files.push_back(i->path().filename().string());
    }
    return matching_files;
}
