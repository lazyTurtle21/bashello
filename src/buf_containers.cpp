#include <cstdlib>
#include "../inc/buf_containers.hpp"


command_line::~command_line() {
    delete[]buf;
}


arguments_container::~arguments_container() {
    delete[] buf;
}
