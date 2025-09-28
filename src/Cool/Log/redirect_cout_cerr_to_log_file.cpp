#include "redirect_cout_cerr_to_log_file.hpp"
#include <optional>
#include "Log.hpp"

namespace Cool {

namespace {
class Stringbuf_LogInternal : public std::stringbuf {
public:
    explicit Stringbuf_LogInternal(bool is_cout)
        : _is_cout{is_cout}
    {}

    ~Stringbuf_LogInternal() override
    {
        sync();
    }

    Stringbuf_LogInternal(Stringbuf_LogInternal const&)                = delete;
    Stringbuf_LogInternal& operator=(Stringbuf_LogInternal const&)     = delete;
    Stringbuf_LogInternal(Stringbuf_LogInternal&&) noexcept            = delete;
    Stringbuf_LogInternal& operator=(Stringbuf_LogInternal&&) noexcept = delete;

    int sync() override
    {
        std::string const msg = str();
        if (!msg.empty())
        {
            if (_is_cout)
                Log::internal_info("std::cout", msg);
            else
                Log::internal_warning("std::cerr", msg);
            str("");
        }
        return 0;
    }

private:
    bool _is_cout{};
};

class StreamRedirectionRAII {
public:
    explicit StreamRedirectionRAII(bool is_cout)
        : _buffer{is_cout}
        , _stream{is_cout ? std::cout : std::cerr}
        , _original_buffer{_stream.rdbuf()}
    {
        _stream.rdbuf(&_buffer);
    }

    ~StreamRedirectionRAII()
    {
        _stream.rdbuf(_original_buffer);
    }

    StreamRedirectionRAII(StreamRedirectionRAII const&)                = delete;
    StreamRedirectionRAII& operator=(StreamRedirectionRAII const&)     = delete;
    StreamRedirectionRAII(StreamRedirectionRAII&&) noexcept            = delete;
    StreamRedirectionRAII& operator=(StreamRedirectionRAII&&) noexcept = delete;

public:
    Stringbuf_LogInternal _buffer;
    std::ostream&         _stream;
    std::streambuf*       _original_buffer;
};
} // namespace

static auto cout_redirection_raii() -> std::optional<StreamRedirectionRAII>&
{
    static auto instance = std::optional<StreamRedirectionRAII>{};
    return instance;
}
static auto cerr_redirection_raii() -> std::optional<StreamRedirectionRAII>&
{
    static auto instance = std::optional<StreamRedirectionRAII>{};
    return instance;
}

void start_redirecting_cout_cerr_to_log_file()
{
    cout_redirection_raii().emplace(true /*is_cout*/);
    cerr_redirection_raii().emplace(false /*is_cout*/);
}

void stop_redirecting_cout_cerr_to_log_file()
{
    cout_redirection_raii().reset();
    cerr_redirection_raii().reset();
}

} // namespace Cool