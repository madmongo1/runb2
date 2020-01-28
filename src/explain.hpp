#pragma once

#include "config.hpp"
#include <exception>
#include <ostream>
#include <string>

namespace program {

    struct explainer
    {
        std::exception_ptr ep_;

        static auto
        prepare_buffer(std::size_t level)
        -> std::string &
        {
            static thread_local std::string buffer;
            buffer.clear();
            if (level) buffer += '\n';
            buffer.append(level, ' ');
            return buffer;
        }

        template<class...Stuff>
        static void
        emit(std::ostream &os, std::size_t level, Stuff &&...stuff)
        {
            using expand = bool[];

            void(expand{
                 bool(os << prepare_buffer(level)),
                 bool(os << stuff) ...}
            );
        }

        static void
        report(std::ostream &os, std::size_t level, beast::system_error const &e)
        {
            emit(os, level, "system error: ", e.code().category().name(), " : ", e.code().value(), " : ",
                 e.code().message());
        }

        static void
        report(std::ostream &os, std::size_t level, std::exception const &e)
        {
            emit(os, level, "exception: ", e.what());
        }

        template<class Exception>
        static void
        process(std::ostream &os, Exception &e, std::size_t level = 0)
        {
            using namespace std::literals;

            report(os, level, e);

            try
            {
                std::rethrow_if_nested(e);
            }
            catch (...)
            {
                process(os, std::current_exception(), level + 1);
            }
        }

        static void
        process(std::ostream &os, std::exception_ptr ep, std::size_t level = 0)
        {
            try
            {
                std::rethrow_exception(ep);
            }
            catch (beast::system_error &child)
            {
                process(os, child, level);
            }
            catch (std::exception &child)
            {
                process(os, child, level);
            }
            catch (...)
            {
                emit(os, level + 1, "nonstandard exception");
            }
        }

        friend auto
        operator<<(std::ostream &os, explainer const &ex) -> std::ostream &
        {
            ex.process(os, ex.ep_);

            return os;
        }
    };

    inline auto
    explain(std::exception_ptr ep = std::current_exception()) -> explainer
    {
        return explainer{ep};
    }
}
