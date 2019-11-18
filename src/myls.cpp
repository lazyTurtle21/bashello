#include <iostream>
#include<boost/program_options.hpp>
#include<boost/filesystem.hpp>
#include<map>

#include<vector>
#include<string>
#include <cstring>
#include "../inc/myfile.hpp"


namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef std::map<std::string, std::vector<myfile>> files_map;
typedef std::function<bool(myfile, myfile)> sorting_function;
typedef std::map<char, sorting_function> sorting_map;


struct parameters {
    bool reversed = false, recursive = false, detailed_info = false, additional_info = false;
    std::string sorting{'N'};

    parameters() = default;
};


files_map list_dir(const parameters &args, std::vector<myfile> &dirs, std::string &dir, bool fill_add_info = false) {
    files_map map;
    if (!dir.empty() && dir[dir.size() - 1]=='/')
        dir = dir.substr(0, dir.size() - 1);

    boost::filesystem::recursive_directory_iterator end_itr;
    dirs.emplace_back(myfile(dir, fill_add_info));
    for (boost::filesystem::recursive_directory_iterator i(dir); i != end_itr; ++i) {
        if (!args.recursive && i.depth() >= 1)
            continue;
        std::string parent = i->path().relative_path().remove_filename().string();

        if (fs::is_directory(i->path())) {
            auto d = myfile(i->path().relative_path(), fill_add_info, args.additional_info, args.detailed_info);
            if (args.recursive) {
                map[i->path().relative_path().string()] = {};
                dirs.emplace_back(d);
            }
            map[parent].emplace_back(d);
            continue;
        }

        map[parent].emplace_back(
                myfile(i->path().relative_path(), fill_add_info, args.additional_info, args.detailed_info));
        if (errno)
            return map;
    }
    return map;
}


void sort(files_map &map, std::vector<myfile> &dirs, const std::string &sorting, bool reverse = false) {
    sorting_map sort;
    sort['N'] = [](myfile x, myfile y) {
        if(x.type == "/" && y.type == "/")
            return x.path.relative_path().string() < y.path.relative_path().string();
        return x.name < y.name;
    };
    sort['X'] = [](myfile x, myfile y) { return x.extension.compare(y.extension); };
    sort['t'] = [](myfile x, myfile y) { return x.seconds > y.seconds; };
    sort['S'] = [](myfile x, myfile y) { return x.size > y.size; };
    sort['D'] = [](myfile x, myfile y) { return x.type == "/"; };
    sort['s'] = [](myfile x, myfile y) { return (x.type != "/") && (!x.type.empty()); };

    for (size_t i = 0, end = sorting.size(); i < end; i++) {

        std::sort(dirs.begin(), dirs.end(), sort[sorting[i]]);
        if (reverse)
            std::reverse(dirs.begin(), dirs.end());

        for (auto it = map.begin(); it != map.end(); it++) {
            std::sort(it->second.begin(), it->second.end(), sort[sorting[i]]);
            if (reverse)
                std::reverse(it->second.begin(), it->second.end());

        }
    }
}


void print(files_map &map, std::vector<myfile> &dirs) {
    std::string dir_name;
    std::string file;

    for (size_t i = 0, end = dirs.size(); i < end; i++) {
        dir_name = dirs[i].path.string();
        std::cout << dir_name << ":\n";
        for (int j = 0; j < map[dir_name].size(); j++) {
            std::cout << "  " << map[dir_name][j].get_string() << '\n';
        }
        std::cout << '\n';
    }

}


int main(int argc, char **argv) {
    std::string sort_params;
    po::options_description visible(
            "Usage: myls [path|mask] [-l] [-h|--help] [--sort=U|S|t|X|D|s] [-r] [-F] [-R]\n"
            "Sort entries alphabetically if none of -cftuSUX nor --sort is specified.\n\n"
            "Mandatory arguments to long options are mandatory for short options too.");
    visible.add_options()
            ("help,h", "display this help and exit")
            ("sort", po::value<std::string>(&sort_params), "sort by WORD instead of name: none (-U), size (-S),\n"
                                                           "time (-t), extension (-X)\n"
                                                           "directories (-D), special (-s)")
            (",l", "use a long listing format")
            (",F", "append indicator (one of */=>@|) to entries")
            (",r", "list in reversed order")
            (",R", "list subdirectories recursively");
    po::options_description backend("Backend options");
    backend.add_options()
            ("directory", po::value<std::vector<std::string>>());

    po::options_description all("Allowed options");
    all.add(visible).add(backend);

    po::positional_options_description p;
    p.add("directory", -1);
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
        po::notify(vm);
    } catch (po::error &e) {
        std::cerr << "err: " << e.what() << std::endl << std::endl;
        exit(1);
    }

    if (vm.count("help")) {
        std::cout << visible << std::endl;
        exit(0);
    }

    parameters params;
    std::string posible_params{"DSUXst"};
    std::sort(sort_params.begin(), sort_params.end());


    if (!(std::includes(posible_params.begin(), posible_params.end(), sort_params.begin(), sort_params.end()))) {
        std::cerr << "invalid argument \"" << sort_params << "\" for \"--sort\" \nValid arguments are "
                                                             "U|S|t|X|D|s \nTry 'myls --help' for more information."
                  << std::endl;
        exit(1);
    } else if (!sort_params.empty()) { params.sorting = sort_params; }


    params.recursive = vm.count("-R");
    params.reversed = vm.count("-r");
    params.additional_info = vm.count("-l");
    params.detailed_info = vm.count("-F");

    std::vector<std::string> arg_dirs;
    std::vector<myfile> arg_files;

    if (!vm.count("directory"))
        arg_dirs.emplace_back(".");
    else {
        for (auto &name : vm["directory"].as<std::vector<std::string>>()) {
            if (!boost::filesystem::exists(name)){
                std::cerr << name << " file/directory does not exist\n";
                continue;
            }
            if (boost::filesystem::is_directory(name))
                arg_dirs.emplace_back(name);
            else
                arg_files.emplace_back(myfile(name, true, params.additional_info, params.detailed_info));
        }
    }

    bool fill = vm.count("-F") || vm.count("-l") || vm.count("-r") || vm.count("sort");

    std::vector<myfile> order_dirs;
    for (int j = 0; j < arg_files.size(); j++) {
        std::cout << arg_files[j].get_string() << '\n';
    }

    for (auto &d : arg_dirs) {

        order_dirs.clear();
        errno = 0;
        auto files = list_dir(params, order_dirs, d, fill);
        if (errno) {
            std::cerr << strerror(errno);
            return errno;
        }
        sort(files, order_dirs, params.sorting, params.reversed);
        print(files, order_dirs);
    }

    return 0;
}

