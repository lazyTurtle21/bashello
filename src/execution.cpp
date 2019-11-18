//
// Created by lazyturtle on 18.11.19.
//

#include "../inc/execution.hpp"


int pipe_exec(std::vector<command> &commands, custom_environ &environ_, builtins_map &builtins) {
    for (size_t i = 0; i < commands.size() - 1; ++i) {
        int p[2];
        while (pipe(p) == -1) {}
        commands[i].set_stdout(p[1]);
        commands[i + 1].set_stdin(p[0]);
        commands[i].execute_command(builtins, p[0]);
        close(p[1]);
   }
    return commands.back().execute_command(builtins);
}

std::string magic_transform(std::string input, int &status, custom_environ &environ_, builtins_map &builtins) {
    int p[2];
    while (pipe(p) == -1) {}
    status = prepare_and_execute(std::move(input), environ_, builtins, p[1], p[0]);
    char inbuf[10000];
    read(p[0], inbuf, 10000);
    close(p[0]);
    close(p[1]);
    return inbuf;
}


int prepare_and_execute(std::string string_args, custom_environ &environ_, builtins_map &builtins, int pipe_out,
                        int pipe_in) {

    std::vector<std::string> pipe_args, scripts, default_desc{"", "", ""};
    std::vector<command> commands;
    boost::split(pipe_args, string_args, boost::is_any_of("|"));
    std::string replaced;
    std::pair<std::string, std::vector<std::string>> redirect_files;
    int status = 0;

    for (auto &c: pipe_args) {
        std::vector<std::string> to_redirect;
        replace_vars(c);

        scripts = get_script_vars(c);
        std::cout << scripts.size() << '\n';

        for (auto &s: scripts) {
//            std::cout << "'" << s  <<"'" << '\n';
            auto dd = magic_transform(s, status, environ_, builtins);
//            std::cout << dd << '\n';
            boost::replace_first(c, "$(" + s + ")", dd);
        }
        if (c.find('<') != std::string::npos || c.find('>') != std::string::npos) {
            redirect_files = get_files_for_redirect(c);
            split(redirect_files.first, to_redirect);
        } else {
            split(c, to_redirect);
        }

        status = parse_variables(to_redirect, environ_);
        if (status) {
            std::cerr << ERR_COLOR << "error while creating a new variable: " << strerror(status) << RESET << '\n';
            return status;
        }

        replace_wildcard(to_redirect);
        commands.emplace_back(to_redirect,
                              redirect_files.second.empty() ? default_desc : redirect_files.second, environ_);
    }

    if (commands.empty()) return 0;

    if (commands.size() == 1) {
        if (pipe_out != -1) {
            commands[0].set_stdout(pipe_out);
            status = commands[0].execute_command(builtins, pipe_in);
            close(pipe_out);
        } else
            status = commands[0].execute_command(builtins);
    } else{
        status = pipe_exec(commands, environ_, builtins);
    }

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


