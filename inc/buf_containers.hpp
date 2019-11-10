#ifndef BASHELLO_BUF_CONTAINERS_HPP
#define BASHELLO_BUF_CONTAINERS_HPP


class command_line {
public:
    char * buf = nullptr;

    command_line() = default;

    ~command_line();

};


class arguments_container {
public:
    const char ** buf = nullptr;

    arguments_container() = default;

    ~arguments_container();

};

#endif //BASHELLO_BUF_CONTAINERS_HPP
