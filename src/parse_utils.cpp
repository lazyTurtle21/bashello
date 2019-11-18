#include "../inc/parse_utils.hpp"
#include "../inc/builtins.hpp"


void replace_vars(std::string&command) {
    std::vector<std::string> command_vars;
    std::string var;
    bool write_var = false;
    char *tmp = nullptr;
    for (size_t j = 0; j < command.size(); ++j) {
        if (write_var) {
            if (!isalpha(command[j]) && !isdigit(command[j])) {
                command_vars.push_back(var);
                var.clear();
                write_var = false;
            } else {
                var.push_back(command[j]);
                if (j == command.size() - 1) command_vars.push_back(var);
            }
        } else {
            if (command[j] == '$') write_var = true;
        }
    }
    for (auto &v: command_vars) {
        tmp = getenv(v.c_str());
        boost::replace_first(command, "$" + v, tmp != nullptr ? tmp : "");
    }
}

std::vector<std::string> get_script_vars(std::string& command) {
    std::vector<std::string> scripts;
    std::string script;
    bool write_script = false;
    for (size_t j = 0; j < command.size(); ++j) {
        if (write_script) {
            if (command[j] == ')') {
                scripts.push_back(script);
                script.clear();
                write_script = false;
            } else {
                if(command[j] != '('){
                    script.push_back(command[j]);
                } else{
                    continue;
                }
            }
        } else {
            if (command[j] == '$' && command[j + 1] == '(') write_script = true;
        }
    }
    return scripts;
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
replace_wildcard(std::vector<std::string> &command, std::string &full_path, std::string &dir) {
    std::string filename;
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

std::string remove_leading_ending_spaces(const std::string& str){
    const auto start = str.find_first_not_of(' ');
    const auto end = str.find_last_not_of(' ');
    return str.substr(start, end-start+1);
}

std::pair<std::string, std::vector<std::string>> get_files_for_redirect(std::string cmd) {
    std::vector<std::string>redirect_files = {"", "",""};
    int desc;
    std::vector<int> redirect_signs;
    for (size_t i = 0; i < cmd.size(); ++i) {
        if (cmd[i]=='>' || cmd[i]=='<' ) redirect_signs.emplace_back(i);
    }
    size_t command_end = redirect_signs[0];

    for (size_t j = 0; j < redirect_signs.size(); ++j) {
        std::string file_name;
        size_t ind = redirect_signs[j];
        if (cmd[ind - 1] == '0' || cmd[ind - 1] == '1' || cmd[ind - 1] == '2') {
            desc = cmd[ind - 1] - '0';
            if (j==0) --command_end;
        } else {
            desc = cmd[ind] == '<' ? 0 : 1;
        }

        if (cmd[ind+1] == '&') ++ind;

        if (j == redirect_signs.size()-1){
            file_name = cmd.substr(ind + 1);
        }else{
            size_t end = redirect_signs[j+1];
            if(cmd[redirect_signs[j+1]-1] == '0' || cmd[redirect_signs[j+1]-1] == '1' || cmd[redirect_signs[j+1]-1] == '2') --end;
            file_name = cmd.substr(ind + 1, end - ind - 1);
        }
        redirect_files[desc] = remove_leading_ending_spaces(file_name);
    }
    std::string command = remove_leading_ending_spaces(cmd.substr(0, command_end));
    std::pair<std::string, std::vector<std::string>> new_command = std::make_pair(command, redirect_files);
    return new_command;
}