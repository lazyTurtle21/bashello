#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include "../inc/command.hpp"
#include "../inc/builtins.hpp"
#include "../inc/custom_environ.hpp"
#include "../inc/buf_containers.hpp"
#include "../inc/parse_utils.hpp"

#define ERR_COLOR "\e[31m"
#define DOLLAR "\u001b[38;5;226m"
#define PATH_COLOR "\u001b[38;5;198m"
#define RESET "\u001b[0m"

int pipe_exec(std::vector<command> &commands, custom_environ &environ_, builtins_map &builtins) {
    for (size_t i = 0; i < commands.size() - 1; ++i) {
        int p[2];
//        пайпаєм поки не пайпнеться
        while (pipe(p) == -1) {}
        commands[i].set_stdout(p[1]);
        commands[i + 1].set_stdin(p[0]);
        commands[i].execute_command(builtins, p[0]);
        close(p[1]);
//        я не впевнена чи на наступному кроці треба закривати попередні дескриптори
//        теоретично вони ітак закриються в деструкторі
//        валгрінд ворнінгів не дає, але ви скажіть як думаєте
    }
    return commands.back().execute_command(builtins);
}


int main(int argc, char *argv[], char *envp[]) {
    std::string string_args, full_path, dir, filename;
    std::string current_path = boost::filesystem::current_path().string();
    command_line cmd_line;
    int status = 0;

    std::string pathenv = std::string(getenv("PATH")) + ":" + current_path;
    setenv("PATH", pathenv.c_str(), 1);
    custom_environ environ_(envp);

    builtins_map builtins = get_function(current_path, environ_, status);


    while ((cmd_line.buf = readline((PATH_COLOR + current_path + RESET + DOLLAR + "$ " + RESET).c_str())) != nullptr) {
        if (strlen(cmd_line.buf) > 0) add_history(cmd_line.buf);

        string_args = cmd_line.buf;
        std::vector<std::string> command_;

//        має вертати вектор векторів
        split(string_args, command_);

// TODO:  на цьому кроці треба заміняти $() щоб воно працювало нормально з вар=$()
//        реплейс_варс це то що з =, а парс це то шо заміняє

        replace_vars(command_);
        status = parse_variables(command_, environ_);
        if (status) {
            std::cerr << ERR_COLOR << "error while creating a new variable: " << strerror(status) << RESET << '\n';
            continue;
        }
        if (command_.empty()) continue;

        replace_wildcard(command_, full_path, dir, filename);

// TODO napisat' vector z descriptoramy imenamy
        std::vector<std::string> v{"", "", ""};
        std::vector<std::string> com{"grep", "e"};
        std::vector<std::string> com1{"grep", "b"};
        //pass 1 as 4 parameter if background mode

        //test pipe
        command c(command_, v, environ_);
        command c1(com, v, environ_);
        command c2(com1, v, environ_);

        std::vector<command> commands{c, c1, c2};

//        теоретично має бути так шо якщо там є пайп то вектор буде довним за 2, тоді викликаємо пайп
//        а якщо не довший то можна просто робити екзекют
        status = pipe_exec(commands, environ_, builtins);
//        status = c.execute_command(builtins);
    }

    return 0;
}