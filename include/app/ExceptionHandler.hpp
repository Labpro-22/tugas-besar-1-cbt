#ifndef APP_EXCEPTION_HANDLER_HPP
#define APP_EXCEPTION_HANDLER_HPP

#include <exception>
#include <functional>
#include <iostream>
#include <string>

#include "exception/NimonspoliExceptions.hpp"

class ExceptionHandler {
public:
    template <typename Func>
    static bool guard(const std::string& context, Func&& func) {
        try {
            func();
            return true;
        } catch (const NimonspoliException& e) {
            std::cerr << "[ERROR: " << e.getErrorCode() << "] " << context << ": " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] " << context << ": " << e.what() << "\n";
        } catch (...) {
            std::cerr << "[ERROR] " << context
                      << ": unknown non-standard exception\n";
        }
        return false;
    }
};

#endif
