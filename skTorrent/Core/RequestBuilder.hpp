#ifndef REQUESTBUILDER_HPP
#define REQUESTBUILDER_HPP
#include <concepts>
#include <type_traits>
#include <map>
#include <string>

namespace Torrent {
template <typename T>
concept StringLike = requires(T v) { std::string(v); };
;

template <typename T>
concept ToStringable = requires(T v) {
    { std::to_string(v) } -> std::convertible_to<std::string>;
};

template <typename T>
concept CanConvertToStr = StringLike<T> || ToStringable<T>;

class RequestBuilder
{
public:
    RequestBuilder()  = default;
    ~RequestBuilder() = default;

    template <CanConvertToStr T>
    void addParameter(const std::string& key, const T& value)
    {
        if constexpr (StringLike<T>)
        {
            m_parameters[key] = value;
        }
        else
        {
            m_parameters[key] = std::to_string(value);
        }
    }

    void setUrl(const std::string_view& url)
    {
        m_url = url;
    }

    std::string build()
    {
        std::string req  = m_url;
        req             += "?";
        for (const auto& [key, value] : m_parameters)
        {
            req += key + "=" + value + "&";
        }
        req.pop_back();
        return req;
    }

    std::string m_url;
    std::map<std::string, std::string> m_parameters;
};

}  // namespace Torrent
#endif  // REQUESTBUILDER_HPP
