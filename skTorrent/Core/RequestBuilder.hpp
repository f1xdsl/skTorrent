#ifndef REQUESTBUILDER_HPP
#define REQUESTBUILDER_HPP
#include <concepts>
#include <type_traits>
#include <unordered_map>
#include <string>

#include <Logger.hpp>

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

struct RequestParams
{
    void reset()
    {
        m_parameters.clear();
    }

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

    std::unordered_map<std::string, std::string>& get()
    {
        return m_parameters;
    }

    std::unordered_map<std::string, std::string> m_parameters;
};

class RequestBuilder

{
public:
    RequestBuilder()                      = default;
    ~RequestBuilder()                     = default;
    RequestBuilder(const RequestBuilder&) = delete;

    RequestBuilder(const std::string& url)
        : m_url(url)
    {}

    void resetParameters()
    {
        m_parameters.reset();
    }

    void setUrl(const std::string_view& url)
    {
        m_url = url;
    }

    template <CanConvertToStr T>
    void addParameter(const std::string& key, const T& value)
    {
        m_parameters.addParameter(key, value);
    }

    std::string build()
    {
        if (m_url.empty())
        {
            LOG_ERROR(RequestBuilder, "URL is empty");
            return "";
        }

        std::string req  = m_url;
        req             += "?";
        for (const auto& [key, value] : m_parameters.get())
        {
            req += key + "=" + value + "&";
        }
        req.pop_back();
        LOG_INFO(RequestBuilder, "Built request", LOG_MD(Request, req));
        return req;
    }

    std::string m_url;
    RequestParams m_parameters;
};

}  // namespace Torrent
#endif  // REQUESTBUILDER_HPP
