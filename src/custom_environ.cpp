#include <iostream>
#include <sstream>
#include "../inc/custom_environ.hpp"


custom_environ::custom_environ(char **envp) {
    int i = 0;
    while (envp[i]) {
        int len = strlen(envp[i]);
        char *element = new char[len + 1];
        std::copy(envp[i], envp[i] + len + 1, element);
        env.push_back(element);
        i++;
    }
    env.push_back(nullptr);
}

custom_environ::~custom_environ() {
    for (auto &element: env)
        delete[] element;
}

char **custom_environ::c_arr() {
    return env.empty() ? nullptr : env.data();
}

void custom_environ::set(std::string name, std::string value) {
    char *element = new char[name.size() + value.size() + 2];
    std::copy(&name[0], &name[0] + name.size(), element);
    element[name.size()] = '=';
    std::copy(&value[0], &value[0] + value.size() + 1, element + name.size() + 1);
    for (size_t i = 0; i < env.size() - 1; ++i) {
        if (boost::find_first(env[i], name)) {
            delete[] env[i];
            env[i] = element;
            return;
        }
    }
    env.insert(env.end() - 1, element);
}

std::string custom_environ::to_string() {
    std::stringstream ss;
    for (auto &el: env) {
        ss << el << std::endl;
    }
    return ss.str();
}


