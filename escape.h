#ifndef ESCAPE_H
#define ESCAPE_H

#include <string>

namespace escape {

    /*
     * We could achieve the same result in a more elegant way e.g. using
     * the std::regex library, but doing it the "old-fashioned" way seems
     * to be faster by about a factor of 20.
     */
    static std::string escapeSpecialChars(const std::string &input) {
        static const std::string escapeUs{"\r\n\\"};
        std::string result;
        for (char c : input) {
            if (escapeUs.find(c) != std::string::npos)
                result.push_back('\\');
            result.push_back(c);
        }
        return result;
    }
}

#endif //ESCAPE_H
