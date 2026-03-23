#ifndef CLI_INPUT_HPP
#define CLI_INPUT_HPP

#include <string>
#include <iostream>
#include <limits>

namespace cli {

    inline std::string getLine(const std::string & prompt) {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        return value;
    }
    
    inline bool confirm(const std::string & prompt) {
        std::string s = cli::getLine(prompt + " (y/n): ");
        return (s == "y" || s == "Y" || s == "yes" || s == "YES" || s == "Yes");
    }

    template<typename T>
    T parseNumber(const std::string & s) {
        if constexpr (std::is_same_v<T, uint64_t>)
            return static_cast<T>(std::stoull(s));
        else if constexpr (std::is_same_v<T, uint32_t>)
            return static_cast<T>(std::stoul(s));
        else if constexpr (std::is_same_v<T, int>)
            return static_cast<T>(std::stoi(s));
        else if constexpr (std::is_same_v<T, long>)
            return static_cast<T>(std::stol(s));
    }

    template<typename T>
    bool getNumber(const std::string & prompt, T & out) {

        std::string s = getLine(prompt);

        try {
            out = parseNumber<T>(s);
            return true;
        }
        catch (...) {
            std::cout << "Invalid input\n";
            return false;
        }
    }

    inline void clearInput() {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

}

#endif
