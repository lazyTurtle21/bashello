#include <cstdlib>
#include "../inc/buf_containers.hpp"


command_line::~command_line() {
    if (buf != nullptr) free(buf);
}


arguments_container::~arguments_container() {
    if (buf != nullptr) free(buf);
}
