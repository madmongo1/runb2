#include "explain.hpp"

#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>
#include <range/v3/all.hpp>

namespace program
{
    struct b2_not_found : std::runtime_error
    {
        b2_not_found(fs::path const &initial)
        : runtime_error("b2 not found in any directory in or above "s + initial.string< std::string >())
        {
        }
    };

    struct matches
    {
        matches(fs::path candidate)
        : candidate_(std::move(candidate))
        {
        }

        bool operator()(std::string_view x) const { return boost::iequals(x, candidate_.string()); }

        fs::path candidate_;
    };

    auto is_b2(fs::path const &pathname) -> bool
    {
        using namespace ranges::v3;

        return fs::status(pathname).type() == fs::file_type::regular_file and
               any_of({ "b2.exe"sv, "b2"sv }, matches(fs::basename(pathname)));
    }

    auto home_dir() -> fs::path
    try
    {
        auto make_path = [](auto... ps) {
            auto result = std::optional< fs::path >();
            if ((ps and ...))
                result.emplace((fs::path(ps) / ...));
            return result;
        };

        auto result = make_path(std::getenv("HOME"));
        if (not result)
            result = make_path(std::getenv("USERPROFILE"));
        if (not result)
            result = make_path(std::getenv("HOMEDRIVE"), std::getenv("HOMEPATH"));
        return *result;
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error("HOME environment variable not set"));
    }

    auto require_private_store() -> fs::path
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

    auto find_b2(fs::path initial) -> fs::path
    {
        if (not initial.is_absolute())
            throw std::invalid_argument("find_b2: path not absolute: "s + initial.string< std::string >());

        auto current = initial;
        while (1)
        {
            for (auto &&entry : fs::directory_iterator(current))
            {
                if (auto candidate = entry.path(); is_b2(candidate))
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
        dump(std::vector< std::string > const &args)
        : args(args)
        {
        }

        friend std::ostream &operator<<(std::ostream &os, dump const &d)
        {
            ranges::v3::for_each(d.args, [&os, sep = ""](auto &&arg) mutable {
                os << sep << std::quoted(arg);
                sep = " ";
            });
            return os;
        }

        std::vector< std::string > const &args;
    };

    auto transform_args(std::vector< std::string > &input) -> std::vector< char * >
    {
        using namespace ranges;

        return view::concat(input | view::transform(data), view::single(static_cast< char * >(nullptr))) | to_vector;
    }

    auto open_out(fs::path const &p) -> std::ofstream
    {
        auto ofs = std::ofstream(p.string());
        ofs.exceptions(std::ios::badbit | std::ios::failbit);
        return ofs;
    }

    auto open_in(fs::path const &p) -> std::ifstream
    {
        auto ifs = std::ifstream(p.string());
        if (ifs.bad())
            throw std::runtime_error("no such store to load from: "s + p.string());
        return ifs;
    }

    template < class Stream >
    auto to_strings(Stream &&stream)
    {
        auto result = std::vector< std::string >();

        while ((stream >> std::quoted(result.emplace_back())) or (result.pop_back(), false))
            ;

        return result;
    }

    template < class Vec1, class... Vecs >
    auto join(Vec1 &&vec, Vecs &&... vecs)
    {
        auto result = std::forward< Vec1 >(vec);

        auto next = [&](auto &&source) {
            if constexpr (std::is_rvalue_reference_v< decltype(source) >)
            {
                std::move(source.begin(), source.end(), std::back_inserter(result));
            }
            else
            {
                std::copy(source.begin(), source.end(), std::back_inserter(result));
            }
        };

        (next(std::forward< Vecs >(vecs)), ...);

        return result;
    }

    namespace po = boost::program_options;

    int run(po::variables_map const &options, std::vector< std::string > b2_options)
    {
        if (options.count("load"))
        {
            // do load stuff
            auto load_path = require_private_store() / options.at("load").as< std::string >();
            std::cout << "loading from " << load_path << '\n';
            b2_options = join(to_strings(open_in(load_path)), std::move(b2_options));
        }

        if (options.count("store"))
        {
            // do store stuff
            auto storage_path = require_private_store() / options.at("store").as< std::string >();
            std::cout << "storing in " << storage_path << '\n';
            open_out(storage_path) << dump(b2_options);
        }

        if (options.count("noexec"))
        {
            std::cout << "not executing: b2 " << dump(b2_options) << '\n';
        }
        else
        {
            auto b2_exe = find_b2(fs::current_path());
            std::cout << "executing: " << b2_exe << " " << dump(b2_options) << '\n';
            auto result = ::execv(b2_exe.string< std::string >().c_str(), transform_args(b2_options).data());
            if (result == -1)
                throw std::runtime_error("failed to launch b2!");
        }

        return 0;
    }

}   // namespace program

int main(int argc, char **argv)
{
    using namespace program;

    try
    {
        auto cmdline_desc = po::options_description();

        cmdline_desc.add_options()(
            "store", po::value< std::string >(), "store the given b2 command line in slot <string> and execute")(
            "load", po::value< std::string >(), "load the given b2 command line from slot <string> and execute")(
            "noexec", "set to prevent the launch of b2")("help", "show this help");

        po::variables_map vm;
        auto cmdline_opts = po::command_line_parser(argc, argv).options(cmdline_desc).allow_unregistered().run();
        po::store(cmdline_opts, vm);
        auto b2_options = po::collect_unrecognized(cmdline_opts.options, po::include_positional);

        if (vm.count("help"))
        {
            std::cout << "usage: runb2 [options...] [options to pass to b2]\n";
            std::cout << "\nValid Options:\n" << cmdline_desc << '\n';
            return 0;
        }

        return program::run(vm, b2_options);
    }
    catch (...)
    {
        std::cerr << program::explain() << '\n';
        return 127;
    }
}
