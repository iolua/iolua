#ifndef LEMON_FS_FILEPATH_HPP
#define LEMON_FS_FILEPATH_HPP


#include <list>
#include <regex>
#include <string>
#include <locale>
#include <codecvt>
#include <utility>
#include <ostream>
#include <sstream>
#include <cstddef>
#include <system_error>


#include <lemon/config.h>

namespace lemon{ namespace fs{

    #ifdef WIN32
    using char_type = wchar_t;

        constexpr char_type os_separator = '\\';
    #else
    using char_type = char;

    constexpr char_type os_separator = '/';
    #endif

    constexpr char_type slash = '/';

    inline bool is_separator(char_type c)
    {
        return c == char_type('\\') || c == char_type('/');
    }

    #ifdef WIN32
    using string_convert = std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>, wchar_t> ;
    #else
    using string_convert = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> ;
    #endif

    using string_type = std::basic_string<char_type>;

    /*
     * the filepath object
     */
    class filepath
    {
    public:
        filepath(){}
        /**
         * create new filepath object by filepath string
         * @source the filepath string presentation
         */
        filepath(const string_type & source)
        {
            parse(source);
        }

    #ifdef WIN32
        filepath(const std::string & source)
            {
                string_convert conv;

                parse(conv.from_bytes(source));
            }

            filepath(const char* source)
            {
                string_convert conv;

                parse(conv.from_bytes(source));

            }
    #endif

        filepath(const char_type* source)
        {
            parse(source);
        }

        filepath(const filepath & source)
            :_volume(source._volume),_nodes(source._nodes)
        {

        }

        filepath(filepath && rhs)
            :_volume(rhs._volume),_nodes(rhs._nodes)
        {

        }

        template <typename Volume ,typename Iter>
        filepath(Volume && name, Iter begin,Iter end)
            :_volume(std::forward<Volume>(name)),_nodes(begin,end)
        {

        }

        filepath & operator = (const filepath & rhs)
        {
            _volume = rhs._volume;
            _nodes = rhs._nodes;
            return *this;
        }

        filepath & operator = (const filepath && rhs)
        {
            _volume = rhs._volume;
            _nodes = rhs._nodes;
            return *this;
        }

        template<typename Source>
        filepath & operator = (const Source & source)
        {
            *this = filepath(source);

            return *this;
        }


        filepath & append(const filepath & rhs)
        {
            if(rhs.empty())
            {
                return *this;
            }

            if(rhs._nodes.front() == string_type(1,'/'))
            {

            }

            _nodes.insert(_nodes.end(),rhs._nodes.begin(),rhs._nodes.end());

            return *this;
        }


        filepath & operator /= (const filepath &rhs)
        {
            return append(rhs);
        }

        filepath  operator / (const filepath &rhs) const
        {
            filepath path = *this;

            return path.append(rhs);
        }

        void clear()
        {
            _nodes.clear();
            _volume.clear();
        }

        bool empty() const
        {
            return _nodes.empty() && _volume == string_type();
        }

        filepath parent_path() const
        {
            if(empty())
            {
                return *this;
            }

            auto path = *this;

            path._nodes.pop_back();

            return path;
        }

        filepath root_name() const
        {
            auto path = *this;

            path._nodes.clear();

            return path;
        }

        filepath root_directory() const
        {
            if(empty())
            {
                return filepath();
            }

            if(_nodes.front() == string_type(1,'/'))
            {
                filepath path;

                path._nodes.push_back(_nodes.front());

                return path;
            }

            return filepath();
        }

        filepath filename() const
        {
            if(_nodes.size() == 2 && _nodes.front() == string_type(1,'/') && _nodes.back() == string_type(1,'.'))
            {
                return filepath(string_type(1,'/'));
            }

            if(_nodes.empty())
            {
                return filepath();
            }

            return filepath(_nodes.back());
        }


        filepath extension() const
        {
            auto name = filename().string();

            std::regex expr("^(.*)(\\.[^\\.]*)$");
            std::smatch match;

            if(!std::regex_match(name,match,expr))
            {
                return filepath(string_type());
            }

            return filepath(match[2]);
        }

        filepath stem() const
        {
            auto name = filename().string();

            std::regex expr("^(.*)(\\.[^\\.]*)$");
            std::smatch match;

            if(!std::regex_match(name,match,expr))
            {
                return filepath(string_type());
            }

            return filepath(match[1]);
        }

        bool has_root_directory() const
        {
            return !root_directory().empty();
        }

        bool has_filename() const
        {
            return false;
        }

        bool has_parent() const
        {
            return !parent_path().empty();
        }

        std::string string() const
        {
            auto str = to_string(os_separator);

    #ifdef WIN32
            string_convert conv;
                    return conv.to_bytes(str);
    #else
            return str;
    #endif //WIN32
        }

        std::wstring wstring() const
        {
            auto str = to_string(os_separator);

    #ifdef WIN32
            return str;
    #else
            string_convert convert;
            return convert.from_bytes(str);
    #endif //WIN32
        }

        std::string generic_string() const
        {
            auto str = to_string(slash);

    #ifdef WIN32
            string_convert conv;
                    return conv.to_bytes(str);
    #else
            return str;
    #endif //WIN32
        }

        std::wstring generic_wstring() const
        {
            auto str = to_string(slash);

    #ifdef WIN32
            return str;
    #else
            string_convert conv;
            return conv.from_bytes(str);
    #endif //WIN32
        }

    private:

        string_type to_string(char_type separator) const
        {
            std::basic_stringstream<char_type > stream;

            stream << _volume;

            if(_nodes.empty())
            {
                return stream.str();
            }

            auto iter = _nodes.begin();

            if(_nodes.front() == string_type(1,'/'))
            {
                if(_nodes.size() == 1)
                {
                    stream << separator;

                    return stream.str();
                }
            }
            else
            {
                stream << *iter;
            }

            iter ++;


            for(;iter != _nodes.end(); iter++)
            {
                if(*iter == string_type(1,'/')) continue;

                stream << separator << *iter;
            }

            auto str = stream.str();

            if(_nodes.back() == string_type(1,'.') && _nodes.size() > 1)
            {
                str = str.substr(0,str.length() - 1);
            }

            return str;
        }

        void parse(const string_type & source)
        {
            clear();

            parseVolume(source);

            std::size_t start = 0,curr = 0;

            auto body = source.substr(_volume.length());

            if(body.empty())
            {
                return;
            }

            if(body[0] == char_type('\\') || body[0] == char_type('/'))
            {
                _nodes.push_back(string_type(1,slash));
            }

            for(auto & c : body)
            {
                if (c == char_type('\\') || c == char_type('/'))
                {
                    auto node  = body.substr(start,curr - start);

                    if (!node.empty())
                    {
                        _nodes.push_back(node);
                    }

                    start = curr + 1;
                }

                curr ++;
            }

            // push last string
            if(start < body.length())
            {
                _nodes.push_back(body.substr(start));
            }
            else if(!_nodes.empty())
            {
                _nodes.push_back(string_type(1,'.'));
            }

        }

        inline void parseVolume(const string_type & source)
        {
            if(source.length() < 2)
            {
                return;
            }

            auto label = source[0];

            if (source[1] == (':') &&
                ((char_type('a') <= label && label <= char_type('z'))
                    || (char_type('A') <= label && label <= char_type('Z'))))
            {
                _volume = source.substr(0,2);
                return;
            }

            else if(source.length() > 5) //UNC
            {
                if(is_separator(source[0]) && is_separator(source[1]) && !is_separator(source[2]) && source[2] != char_type('.'))
                {
                    for(size_t i = 4; i < source.length(); i ++)
                    {
                        if (is_separator(source[i]))
                        {
                            i ++;

                            if(!is_separator(source[i]))
                            {
                                if(source[i] == char_type('.'))
                                {
                                    return;
                                }

                                for(std::size_t j = i; j < source.length(); j ++)
                                {
                                    if(is_separator(source[j]))
                                    {
                                        _volume = source.substr(0,j);

                                        return;
                                    }
                                }

                                _volume = source;

                                return;
                            }


                        }
                    }
                }
            }
        }

    private:
        string_type                         _volume;
        std::list<string_type>              _nodes;
    };

    inline std::ostream & operator << (std::ostream & stream, const filepath & path)
    {
        stream << path.string();

        return stream;
    }
}}

#endif //LEMON_FS_FILEPATH_HPP

