#ifndef LEMON_STRINGS_HPP
#define LEMON_STRINGS_HPP
#include <regex>
#include <vector>
#include <sstream>
#include <string>
#include <unordered_map>
namespace lemon { namespace strings {


    inline std::string escapeChar(char c)
    {
        const static std::unordered_map<char, std::string> ScapedSpecialCharacters = {
            {'.', "\\."}, {'|', "\\|"}, {'*', "\\*"}, {'?', "\\?"},
            {'+', "\\+"}, {'(', "\\("}, {')', "\\)"}, {'{', "\\{"},
            {'}', "\\}"}, {'[', "\\["}, {']', "\\]"}, {'^', "\\^"},
            {'$', "\\$"}, {'\\', "\\\\"}
        };

        auto it = ScapedSpecialCharacters.find(c);

        if (it == ScapedSpecialCharacters.end())
            return std::string(1, c);

        return it->second;
    }

    template <typename Char> inline std::basic_string<Char> escapeString(const std::basic_string<Char> & str) {

        std::stringstream stream;

        std::for_each(begin(str), end(str), [&stream](const char character) {
            stream << escapeChar(character);
        });

        return stream.str();
    }

    template <typename Char> inline std::vector<std::basic_string<Char>> escapeStrings(const std::vector<std::basic_string<Char>> & strs) {

        std::vector<std::basic_string<Char>> result = strs;

        for(auto iter = begin(result); iter != end(result); iter ++ )
        {
            *iter = escapeString(*iter);
        }

        return strs;
    }



    template <typename Char>
    inline std::basic_string<Char> join(
        const std::vector<std::basic_string<Char>> & tokens,
        const std::basic_string<Char> & delimiter) {

        std::basic_stringstream<Char> stream;

        stream << tokens.front();

        std::for_each(
            begin(tokens) + 1,
            end(tokens),
            [&](const std::basic_string<Char> &elem) { stream << delimiter << elem; }
        );

        return stream.str();
    }

    /**
     * split string by delimiters
     */
    template <typename Char>
    inline std::vector<std::basic_string<Char>> split(
        const std::basic_string<Char> & str,
        const std::vector<std::basic_string<Char>> & delimiters)
    {

        auto regexStr = join(escapeStrings(delimiters), std::basic_string<Char>(1,Char('|')));

        std::basic_regex<Char> rgx(regexStr);

        std::regex_token_iterator<typename std::basic_string<Char>::const_iterator>
            first{begin(str), end(str), rgx, -1},
            last;

        return {first, last};
    }

    /**
    * split string by delimiters
    */
    template <typename Char>
    inline std::vector<std::basic_string<Char>> split(
        const std::basic_string<Char> & str,
        const std::basic_string<Char> & delimiter)
    {
        std::vector<std::string> delimiters = {delimiter};

        return split(str,delimiters);
    }

    template <typename Char>
    inline std::vector<std::basic_string<Char>> split(
        const Char* str,
        const Char* delimiter)
    {
        return split(str,delimiter);
    }
}}

#endif //LEMON_STRINGS_HPP