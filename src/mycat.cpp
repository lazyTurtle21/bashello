#include <iostream>
#include <boost/program_options.hpp>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <sstream>


int write(int fd, const char* buffer , ssize_t size , int* status){
    ssize_t read_bytes = 0;
    while( read_bytes < size ) {
        ssize_t written_now = write(fd, buffer + read_bytes, size - read_bytes);
        if( written_now == -1){
            if (errno == EINTR)
                continue;
            else{
                *status = errno;
                return -1;
            }
        } else {
            read_bytes += written_now;
        }
    }
    return 0;
}


void exit_with_error(std::string& error_text){
    int status = 0;
    size_t text_size = error_text.size();
    write(2, error_text.c_str(), text_size, &status);
    _exit(errno);

}


bool file_exists(std::string &filepath){
    return boost::filesystem::exists(filepath);
}


namespace po = boost::program_options;


int main(int argc, char const *argv[]) {
    bool hexik = false;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("A", "prints invisible symbols as it's hexadecimal codes")
            ("input-file", po::value<std::vector<std::string> >(), "input file");

    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(desc).positional(p).style(po::command_line_style::default_style |
                                              po::command_line_style::allow_long_disguise).run(), vm);
    po::notify(vm);
    std::vector<std::string> filenames;
    if (vm.count("input-file")) {
        for (const std::string &i: vm["input-file"].as<std::vector<std::string>>()) {
            filenames.push_back(i);
        }
    }
    if (vm.count("help")) {
        std::cout << desc << std::endl;
    }

    if (vm.count("A")) {
        hexik = true;
    }


    for (size_t i = 0, len = filenames.size(); i < len; i++) {
        if (!file_exists(filenames[i])) {
            errno = ENOENT;
            std::string error_description = "Error code " + std::to_string(errno) +
                                            ": File " + filenames[i] + " does not exist.\n";
            exit_with_error(error_description);
        }
    }

    std::vector<int> descriptors;
    for (size_t i = 0, len = filenames.size(); i < len; i++) {
        int file_desc = open(filenames[i].c_str(), O_RDONLY);
        if (file_desc == -1) {
            std::string error_description = filenames[i] + ": " + strerror(errno);
            exit_with_error(error_description);
        }
        descriptors.emplace_back(file_desc);
    }

    size_t buf_size = 1024 * 1024;
    char *buf = new char[buf_size];
    char *buf2 = nullptr;

    if (hexik) {
        size_t buf_size2 = 4 * buf_size;
        buf2 = new char[buf_size2];
    }
    int status = 0;
    for (size_t i = 0, len_desc = descriptors.size(); i < len_desc; i++) {
        bool file_is_read = false;
        while (!file_is_read) {
            ssize_t read_bytes = 0;
            auto counter = 0;
            while (read_bytes < buf_size) {
                ssize_t success = read(descriptors[i], buf + read_bytes,
                                       buf_size - read_bytes);
                if (success == -1) {
                    if (errno != EINTR){
                        std::string error_text = strerror(errno);
                        exit_with_error(error_text);
                    }
                } else if (success == 0) {
                    file_is_read = true;
                    break;
                } else {
                    read_bytes += success;
                }
                if (hexik) {
                    for (size_t j = read_bytes - success; j < read_bytes; j++) {
                        if (!isprint(buf[j]) && !isspace(buf[j])) {
                            char hexik_str[5];
                            sprintf(hexik_str, "%s%02X", "\\x",  (unsigned char) buf[j]);
                            for (int k = 0; k < 4; k++) {
                                buf2[counter + k] = hexik_str[k];
                            }
                            counter += 4;
                        } else {
                            buf2[counter] = buf[j];
                            counter += 1;
                        }
                    }
                }
            }
            if (hexik) {
                write(1, buf2, counter, &status);
            } else {
                write(1, buf, read_bytes, &status);
            }
        }
    }
    int closed;
    for (auto &fd: descriptors) {
        while(true) {
            closed = close(fd);
            if (closed == -1) {
                if (errno != EINTR) {
                    std::string err_txt = strerror(errno);
                    exit_with_error(err_txt);
                }
            } else{
                break;
            }
        }
    }
    delete[] buf;
    if (hexik)
        delete[] buf2;

    return 0;

}
