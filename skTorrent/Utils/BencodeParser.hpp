#ifndef BENCODEPARSER_HPP
#define BENCODEPARSER_HPP
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <stdexcept>
#include <cstdint>
#include <cctype>

namespace Torrent::Utils::Bencode {
struct Value;
using List    = std::vector<Value>;
using Dict    = std::map<std::string, Value>;
using Integer = uint64_t;
using String  = std::string;

struct Value: std::variant<Integer, String, List, Dict>
{
    using variant::variant;

    bool isInt() const
    {
        return std::holds_alternative<Integer>(*this);
    }

    bool isStr() const
    {
        return std::holds_alternative<String>(*this);
    }

    bool isList() const
    {
        return std::holds_alternative<List>(*this);
    }

    bool isDict() const
    {
        return std::holds_alternative<Dict>(*this);
    }

    const Integer& asInt() const
    {
        return std::get<Integer>(*this);
    }

    const String& asStr() const
    {
        return std::get<String>(*this);
    }

    const List& asList() const
    {
        return std::get<List>(*this);
    }

    const Dict& asDict() const
    {
        return std::get<Dict>(*this);
    }
};

class Parser
{
public:
    explicit Parser(std::string_view data)
        : m_data(data)
    {}

    Value parse()
    {
        return parseValue();
    }

private:
    std::string_view m_data;
    size_t m_pos = 0;

    char peek() const
    {
        if (m_pos >= m_data.size())
        {
            throw std::runtime_error("Unexpected end of data");
        }
        return m_data[m_pos];
    }

    char get()
    {
        char c = peek();
        ++m_pos;
        return c;
    }

    Value parseValue()
    {
        char c = peek();
        if (c == 'i')
        {
            return parseInt();
        }
        if (c == 'l')
        {
            return parseList();
        }
        if (c == 'd')
        {
            return parseDict();
        }
        if (std::isdigit(c))
        {
            return parseString();
        }
        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

    Value parseInt()
    {
        get();  // 'i'
        size_t start = m_pos;
        while (peek() != 'e')
        {
            get();
        }
        std::string numStr(m_data.substr(start, m_pos - start));
        get();  // 'e'
        return Value{std::stoull(numStr)};
    }

    Value parseString()
    {
        size_t start = m_pos;
        while (std::isdigit(peek()))
        {
            get();
        }
        std::string lenStr(m_data.substr(start, m_pos - start));
        size_t len = std::stoul(lenStr);

        if (get() != ':')
        {
            throw std::runtime_error("Expected ':' in string");
        }

        if (m_pos + len > m_data.size())
        {
            throw std::runtime_error("String out of range");
        }

        std::string s(m_data.substr(m_pos, len));
        m_pos += len;
        return Value{s};
    }

    Value parseList()
    {
        get();  // 'l'
        List list;
        while (peek() != 'e')
        {
            list.push_back(parseValue());
        }
        get();  // 'e'
        return Value{list};
    }

    Value parseDict()
    {
        get();  // 'd'
        Dict dict;
        while (peek() != 'e')
        {
            Value key = parseString();
            if (!key.isStr())
            {
                throw std::runtime_error("Dictionary key must be string");
            }
            Value val         = parseValue();
            dict[key.asStr()] = val;
        }
        get();  // 'e'
        return Value{dict};
    }
};

}  // namespace Torrent::Utils::Bencode
#endif  // BENCODEPARSER_HPP
