#include <iostream>
#include "explain.hpp"
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <array>
#include <range/v3/all.hpp>
#include <boost/core/ignore_unused.hpp>
#include <iomanip>

namespace program {

struct b2_not_found
    : std::runtime_error
{
    b2_not_found(fs::path const &initial)
        : std::runtime_error::runtime_error(
        "b2 not found in any directory in or above "s + initial.string<std::string>())
    {}
};

auto
is_b2(fs::path const &pathname) ->
bool
{
    auto stat = fs::status(pathname);
    if (stat.type() != fs::file_type::regular_file)
        return false;

    auto name = pathname.filename().string<std::string>();

    static const auto candidates = std::array{
        "b2.exe"sv,
        "b2"sv
    };

    auto name_matches = [name = fs::basename(pathname)](auto &&candidate) {
        return boost::iequals(name, candidate);
    };

    return ranges::v3::any_of(candidates, name_matches);
}

auto
home_dir()
-> fs::path
try
{
    auto make_path = [](auto...ps) {
        auto result = std::optional<fs::path>();
        if ((ps and ...))
            result.emplace((fs::path(ps) / ...));
        return result;
    };

    auto result = make_path(std::getenv("HOME"));
    if (not result) result = make_path(std::getenv("USERPROFILE"));
    if (not result) result = make_path(std::getenv("HOMEDRIVE"), std::getenv("HOMEPATH"));
    return *result;
}
catch (...)
{
    std::throw_with_nested(std::runtime_error("HOME environment variable not set"));
}

auto
require_private_store()
-> fs::path
try
{
    auto result = home_dir() / ".runb2";

    if (not fs::exists(result))
    {
        fs::create_directories(result);
    }

    return result;
}
catch (...)
{
    std::throw_with_nested(std::runtime_error("require_private_store: failed"));
}

auto
find_b2(fs::path initial)
-> fs::path
{
    if (not initial.is_absolute())
        throw std::invalid_argument("find_b2: path not absolute: "s + initial.string<std::string>());

    auto current = initial;
    while (1)
    {
        auto first = fs::directory_iterator(current);
        auto last = fs::directory_iterator();
        for (; first != last; ++first)
        {
            auto candidate = first->path();
            if (is_b2(candidate))
            {
                return candidate;
            }
        }

        if (current.has_parent_path())
        {
            current = current.parent_path();
        }
        else
        {
            throw b2_not_found(initial);
        }
    }
}

struct dump
{
    dump(std::vector<std::string> const &args) : args(args)
    {}

    friend std::ostream &
    operator<<(
        std::ostream &os,
        dump const &d)
    {
        ranges::v3::for_each(d.args, [&os, sep = ""](auto &&arg) mutable {
            os << sep << std::quoted(arg);
            sep = " ";
        });
        return os;
    }

    std::vector<std::string> const &args;
};

namespace po = boost::program_options;

int
run(
    po::variables_map const &options,
    std::vector<std::string> b2_options)
{
    // first find the b2 program
    auto b2_exe = find_b2(fs::current_path());

    boost::ignore_unused(options, b2_options);

    if (options.count("load"))
    {
        // do load stuff
        auto load_store = options.at("load").as<std::string>();
        auto load_path = require_private_store() / load_store;
        std::cout << "loading from " << load_path << std::endl;
        auto ifs = std::ifstream(load_path.string<std::string>());
        if (ifs.bad())
            throw std::runtime_error("no such store to load from: "s + load_store);
        std::string buffer;
        while (not ifs.eof())
        {
            ifs >> std::quoted(buffer);
            if (ifs)
                b2_options.insert(b2_options.begin(), buffer);
        }
    }

    if (options.count("store"))
    {
        // do store stuff
        auto storage_path = require_private_store() / options.at("store").as<std::string>();
        std::cout << "storing in " << storage_path << std::endl;
        auto ofs = std::ofstream(storage_path.string<std::string>());
        auto sep = "";
        for (auto &&option : b2_options)
        {
            ofs << sep << std::quoted(option);
            sep = " ";
        }
        ofs.close();
    }

    if (not options.count("noexec"))
    {
        std::cout << "executing: " << b2_exe << " " << dump(b2_options) << '\n';
    }

    return 0;
}


} // namespace program

int
main(
    int argc,
    char **argv)
{
    using namespace program;

    try
    {
        auto cmdline_desc = po::options_description();

        cmdline_desc.add_options()
            ("store", po::value<std::string>(), "store the given b2 command line in slot <string> and execute")
            ("load", po::value<std::string>(), "load the given b2 command line from slot <string> and execute")
            ("noexec", po::value<bool>()->implicit_value(true), "set to prevent the launch of b2")
            ("help", "show this help");

        po::variables_map vm;
        auto cmdline_opts = po::command_line_parser(argc, argv).options(cmdline_desc).allow_unregistered().run();
        po::store(cmdline_opts, vm);
        auto b2_options = po::collect_unrecognized(cmdline_opts.options, po::include_positional);

        if (vm.count("help"))
        {
            std::cout << "usage: runb2 [options...] [options to pass to b2]\n";
            std::cout << "\nValid Options:\n" << cmdline_desc << std::endl;
            return 0;
        }

        return program::run(vm, b2_options);
    }
    catch (...)
    {
        std::cerr << program::explain() << std::endl;
        return 127;
    }
}