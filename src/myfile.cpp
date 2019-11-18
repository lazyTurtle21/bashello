//
// Created by lazyturtle on 07.11.19.
//
#include <sys/stat.h> // Структури (типи) для fstat(), lstat (), stat ().
#include <fcntl.h> //
#include <unistd.h>
#include <iostream>

#include "../inc/myfile.hpp"



myfile::myfile(const fs::path &p, bool fill, bool additional_info, bool detailed_info) {
    path = p;
    name = path.filename().string();
    add_info = additional_info;
    det_info = detailed_info;

    if (fill){
        fill_info();
    }
}


std::string myfile::get_string(){
    std::string name_ret = name;
    std::string addit;
    if (add_info) {
        char buff[32];
        strftime(buff, 32, "%Y-%m-%d %H:%M", localtime(reinterpret_cast<const time_t *>(&seconds)));
        addit = std::to_string(size) + ' ' + std::string{buff};
    }
    name_ret = (add_info ? addit + ' ' : "")  +  name + (det_info ? type: "");

    return name_ret;
}


int myfile::fill_info(){
    struct stat sb{};
    if (stat(path.string().c_str(), &sb) == -1) {
        exit(EXIT_FAILURE);
    }
    size = sb.st_size;
    extension = path.extension().string();
    size = sb.st_size;

    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:  case S_IFCHR:  case S_IFREG:  break;
        case S_IFDIR:  type='/';                     break;
        case S_IFIFO:  type='|';                     break;
        case S_IFLNK:  type='@';                     break;
        case S_IFSOCK: type='=';                     break;
        default:       type='?';                     break;
    }
    if (type.empty() & ((sb.st_mode & S_IEXEC) != 0))
        type = '*';

    seconds = sb.st_mtime;
    return 0;
}
