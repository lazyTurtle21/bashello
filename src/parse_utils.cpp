#include "../inc/parse_utils.hpp"
#include "../inc/builtins.hpp"


void replace_vars(std::vector<std::string> &command) {
//     TODO: hz czy tut ale $()
//     треба подумати як працювати в випадку вкладень типу $($var) czy $(echo $(cat $var))
    std::vector<std::string> command_vars;
    std::string var;
    char *tmp = nullptr;
    for (auto &cmd: command) {
        command_vars.clear();
        var.clear();
        bool write_var = false;
        for (int j = 0; j < cmd.size(); ++j) {
            if (write_var) {
                // zabrala duzhki bo v teorii ih ne maye buty vzhe tut
                if (!isalpha(cmd[j])) {
                    command_vars.push_back(var);
                    var.clear();
                    write_var = false;
                } else {
                    var.push_back(cmd[j]);
                    if (j == cmd.size() - 1) command_vars.push_back(var);
                }
            } else {
                if (cmd[j] == '$') write_var = true;
            }
        }
        for (auto &v: command_vars) {
            tmp = getenv(v.c_str());
            boost::replace_first(cmd, "$" + v, tmp != nullptr ? tmp : "");
        }
    }
}

void split(std::string &source, std::vector<std::string> &destination) {
    boost::regex exp("[^ ]*\"([^\"]*)\"[^ ]*|[^\\s]+");
    boost::smatch res;
    boost::sregex_iterator iter1(source.begin(), source.end(), exp), iter2;
    std::string tmp;
    for (; iter1 != iter2; ++iter1) {
        tmp = iter1->str();
        boost::replace_all(tmp, "\"", "");
        destination.push_back(tmp);
    }
}

void
replace_wildcard(std::vector<std::string> &command, std::string &full_path, std::string &dir, std::string &filename) {
    for (int u = 1; u < command.size(); ++u) {
        try {
            full_path = boost::filesystem::absolute(command[u]).string();
        } catch (boost::filesystem::filesystem_error &e) {
            continue;
        }
        auto index = full_path.find_last_of('/');
        dir = full_path.substr(0, index + 1);
        filename = full_path.substr(index + 1, full_path.size());

        auto matches = get_all_files(filename, dir);
        if (!matches.empty()) {
            command[u] = dir + matches[0];
            for (int y = 1; y < matches.size(); ++y) {
                command.insert(command.begin() + u++, dir + matches[y]);
            }
        }
    }
}



int parse_variables(std::vector<std::string>& command, custom_environ &environ_){
    size_t index = 0;
    size_t size = command.size();
    int res = 0;
    std::string name, value;
    while (index < size) {
        size_t found = command[index].find('=');
        if (found == std::string::npos){
            break;
        }
        name = command[index].substr(0, found);
        value = command[index].substr(found + 1);
        res = setenv(name.c_str(), value.c_str(), 1);
        if (res == -1){
            return errno;
        }
        index++;
    }
    while(index--) {
        command.erase(command.begin());
    }
    return 0;

}