#ifndef BASHELLO_CUSTOM_ENVIRON_HPP
#define BASHELLO_CUSTOM_ENVIRON_HPP

#include <vector>
#include <cstring>
#include <string>
#include <boost/algorithm/string.hpp>

class custom_environ {
public:
    explicit custom_environ(char *envp[]);

    ~custom_environ();

    char** c_arr();

    void set(std::string name, std::string value);

    std::string to_string();

private:
    std::vector<char *> env;
};


#endif //BASHELLO_CUSTOM_ENVIRON_HPP
