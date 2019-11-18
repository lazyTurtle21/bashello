//
// Created by lazyturtle on 07.11.19.
//

#ifndef MYLS_MYFILE_HPP
#define MYLS_MYFILE_HPP

#include <string>
#include<boost/filesystem.hpp>

#include<vector>
#include<string>

#include "myfile.hpp"

namespace fs = boost::filesystem;


class myfile {
public:

    std::string name;
    size_t seconds;
    size_t size;
    std::string extension;
    std::string type;
    bool add_info;
    bool det_info;

    fs::path path;


    myfile(const fs::path &path, bool fill=false, bool add_info=false, bool detailed_info=false);
    std::string get_string();

private:
    int fill_info();


};


#endif //MYLS_MYFILE_HPP
