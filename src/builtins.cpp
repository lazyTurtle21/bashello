#include <iostream>
#include <sstream>
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


int help_opts(std::vector<std::string> &command, const std::string &message) {
    optind = 0;
    int c, option_index = 0;
    static struct option opts[] = {
            {"help", no_argument, nullptr, 'h'},
            {nullptr, 0,          nullptr, 0}
    };
    std::vector<const char *> argv(command.size());
    std::transform(command.begin(), command.end(), argv.begin(), [](std::string &str) {
        return str.c_str();
    });
    c = getopt_long(argv.size(), (const_cast<char **>(argv.data())), "h",
                    opts, &option_index);
    if (c == 'h') {
        std::cout << message << std::endl;
        return 1;
    }
    return 0;
}


int mcd(std::vector<std::string> &args, std::string &current_path) {
    int status_ = 0;
    std::stringstream message;
    message << HELP_COLOR << "mcd <path> [-h|--help]\n\tChange the myshell working directory.\n"
                             "\tChange the current directory to DIR.  The default DIR is the value of the\n"
                             "\tHOME myshell variable." << RESET;
    int help = help_opts(args, message.str());
    if (help == 1) return status_;
    if (args.size() == 1) status_ = chdir(getenv("HOME"));
    else status_ = chdir(boost::filesystem::absolute(args[1]).c_str());
    if (status_ == 0) current_path = boost::filesystem::current_path().string();
    return errno;
}


int mexit(std::vector<std::string> &args, int &status) {
    if (args.size() == 1)
        exit(0);

    std::stringstream message;
    message << HELP_COLOR << "mexit [exit code] [-h|--help]\n\tExit the shell.\n"
                             "\tExits the shell with a status of N.  If N is omitted, "
                             "the exit status is that of the last command executed.\n" << RESET;
    int help = help_opts(args, message.str());
    if (help == 1) return 0;

    char *end;
    auto isnumber = strtol(args[1].c_str(), &end, 10);

    if (args.size() == 2 && isnumber)
        exit(atoi(args[1].c_str()));
    return E2BIG;  // argument list too big = 7
}


int mecho(std::vector<std::string> &args) {
    std::stringstream message;
    message << HELP_COLOR << "mecho [text|$<var_name>] [text|$<var_name>]  [text|$<var_name>] ...\n"
                             "\tEcho the STRING(s) to standard output.\n" << RESET << std::endl;
    for (int i = 1; i < args.size(); ++i) std::cout << args[i] << ' ';
    std::cout << std::endl;
    return 0;
}

int mpwd(std::vector<std::string> &args, const std::string &current_path) {
    std::stringstream message;
    message << HELP_COLOR << "mpwd [-h|--help]\n\tPrint the name of the current working directory." << RESET;
    int help = help_opts(args, message.str());
    if (help == 1) return 0;
    std::cout << current_path << std::endl;
    return 0;
}

int mexport(std::vector<std::string> &command, custom_environ &environ_) {
    if (command.size() == 1) return 0;
    std::stringstream message;
    message << HELP_COLOR << "mexport <var_name>[=VAL]\n\tSet export attribute for myshell variables.\n" << RESET;
    int help = help_opts(command, message.str());
    if (help == 1) return 0;

    size_t found = command[1].find('=');
    std::string name = command[1].substr(0, found);
    std::string value = command[1].substr(found + 1);
    int stat = 0;
    if (value.empty()) {
        char *tmp = getenv(name.c_str());
        environ_.set(name, tmp != nullptr ? std::string(tmp) : "");
    } else {
        stat = setenv(name.c_str(), value.c_str(), 1);
        environ_.set(name, value);
        if (stat == -1) {
            return errno;
        }
    }
    return 0;
}

int merrno(std::vector<std::string> &args, int &status) {
    if (args.size() == 1) {
        std::cout << status << '\n';
        return 0;
    }

    if (args[1] == "--help" || args[1] == "-h") {
        std::cout << HELP_COLOR << "mexport <var_name>[=VAL]\n\tSet export attribute for myshell variables.\n" << RESET
                  << std::endl;
        return 0;
    }

    return E2BIG;  // argument list too big = 7
}

std::string magic_transform(std::string input, int &status, custom_environ &environ_, builtins_map &builtins) {
    int p[2];
    while (pipe(p) == -1) {}
    status = prepare_and_execute(input, environ_, builtins, p[1], p[0]);
    char inbuf[10000];
    read(p[0], inbuf, 10000);
    return inbuf;
}

int prepare_and_execute(std::string string_args, custom_environ &environ_, builtins_map &builtins, int pipe_out,
                        int pipe_in) {
    std::string full_path, dir;

    std::vector<std::string> command_;

    split(string_args, command_);
    int status = 0;

    replace_vars(command_);
    status = parse_variables(command_, environ_);
    if (status) {
        std::cerr << ERR_COLOR << "error while creating a new variable: " << strerror(status) << RESET << '\n';
        return status;
    }
    if (command_.empty()) return 0;

    replace_wildcard(command_, full_path, dir);

    std::vector<std::string> v{"", "", ""};
    std::vector<std::string> com{"grep", "e"};
    std::vector<std::string> com1{"grep", "b"};
    //pass 1 as 4 parameter if background mode

    //test pipe
    command c(command_, v, environ_);
    if (pipe_out != -1) {
        c.set_stdout(pipe_out);
        status = c.execute_command(builtins, pipe_in);
        close(pipe_out);
    }
    else
        status = c.execute_command(builtins);
//    command c1(com, v, environ_);
//    command c2(com1, v, environ_);
//
//    std::vector<command> commands{c, c1, c2};

//        теоретично має бути так шо якщо там є пайп то вектор буде довним за 2, тоді викликаємо пайп
//        а якщо не довший то можна просто робити екзекют
//    status = pipe_exec(commands, environ_, builtins);



    return status;
}


int execute_file(std::string &filename, custom_environ &environ_, int &status, builtins_map &builtins) {
    char comment{'#'}, double_quote{'"'}, single_quote{'\''};

    std::ifstream file(filename);

    if (!file.is_open()) {
        status = ENOENT;
        return status;
    }
    std::string line, substr;
    while (getline(file, line)) {
        std::size_t found = line.find(comment);

        while (found != std::string::npos) {
            substr = line.substr(0, found);
            if ((
                    (substr.find(double_quote) != std::string::npos &&
                     line.find(double_quote, found) != std::string::npos)
                    ||
                    (substr.find(single_quote) != std::string::npos &&
                     line.find(single_quote, found) != std::string::npos)
            )) {

                found = line.find(comment, found + 1);
                continue;
            }

            line = line.substr(0, found);
            break;
        }
        if (line.empty())
            continue;
        status = prepare_and_execute(line, environ_, builtins);

    }
    file.close();
    return 0;

}


int execute_file_builtin(std::vector<std::string> &args, custom_environ &environ_, int &status,
                         builtins_map &builtins) {
    if (args.size() == 1) return 0;
    std::stringstream message;
    message << HELP_COLOR << ". <filename>\n\tExecute file in current interpreter.\n" << RESET;
    int help = help_opts(args, message.str());
    if (help == 1) return 0;

    std::string filename;
    if (args.size() == 1) {
        return 0;
    }
    if (args.size() == 2) {
        execute_file(args[1], environ_, status, builtins);
        return status;
    }
    return E2BIG;  // argument list too big = 7
}


builtins_map get_function(std::string &current_path, custom_environ &environ_, int &status) {
    builtins_map map;
    map["mcd"] = [&current_path](std::vector<std::string> command) { return mcd(command, current_path); };
    map["mexport"] = [&environ_](std::vector<std::string> command) { return mexport(command, environ_); };
    map["mpwd"] = [&current_path](std::vector<std::string> command) { return mpwd(command, current_path); };
    map["mexit"] = [&status](std::vector<std::string> command) { return mexit(command, status); };
    map["merrno"] = [&status](std::vector<std::string> command) { return merrno(command, status); };
    map["mecho"] = &mecho;
    map["."] = [&status, &environ_, &map]
            (std::vector<std::string> command) { return execute_file_builtin(command, environ_, status, map); };

    return map;
}




