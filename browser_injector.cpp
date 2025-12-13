#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <string>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;
using json = nlohmann::json;

extern "C" __declspec(dllexport) bool InjectJavaScript(const char* filename)
{
    try
    {
        const char *host = "127.0.0.1";
        const char *port = "27245";

        boost::asio::io_context ioc;

        tcp::resolver resolver{ioc};
        auto const results = resolver.resolve(host, port);

        boost::beast::tcp_stream http_stream{ioc};
        http_stream.connect(results);

        http::request<http::string_body> req{http::verb::get, "/json", 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
        http::write(http_stream, req);

        boost::beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(http_stream, buffer, res);

        http_stream.socket().shutdown(tcp::socket::shutdown_both);

        // Parse JSON and pick a page, should be the first "page" type target
        auto j = json::parse(res.body());

        if (!j.is_array() || j.empty())
        {
            std::cerr << "No targets found\n";
            return 1;
        }

        std::string ws_url;

        for (auto &entry : j)
        {
            if (entry.contains("type") && entry["type"] == "page" &&
                entry.contains("webSocketDebuggerUrl"))
            {
                ws_url = entry["webSocketDebuggerUrl"].get<std::string>();
                break;
            }
        }

        if (ws_url.empty())
        {
            std::cerr << "No page target with webSocketDebuggerUrl\n";
            return 1;
        }

        std::cout << "Using WebSocket URL: " << ws_url << "\n";

        // ws_url format: ws://127.0.0.1:9222/devtools/page/XXXX
        std::string ws_host = host;
        std::string ws_port = port;
        std::string ws_path;

        {
            auto pos = ws_url.find("://");
            auto rest = (pos == std::string::npos) ? ws_url : ws_url.substr(pos + 3);
            // rest = "127.0.0.1:9222/devtools/page/XXXX"
            auto slash_pos = rest.find('/');
            auto hostport = rest.substr(0, slash_pos);
            ws_path = rest.substr(slash_pos); // includes leading '/'

            auto colon_pos = hostport.find(':');
            if (colon_pos != std::string::npos)
            {
                ws_host = hostport.substr(0, colon_pos);
                ws_port = hostport.substr(colon_pos + 1);
            }
            else
            {
                ws_host = hostport;
            }
        }

        tcp::resolver ws_resolver{ioc};
        auto const ws_results = ws_resolver.resolve(ws_host, ws_port);

        websocket::stream<tcp::socket> ws{ioc};
        (void)boost::asio::connect(ws.next_layer(), ws_results);
        std::string host_header = ws_host + ":" + ws_port;
        ws.handshake(host_header, ws_path);

        // Load JavaScript code from file
        std::ifstream jsFile(filename);
        if (!jsFile.is_open()) {
            std::cerr << "Error: Could not open " << filename << "\n";
            return false;
        }

        std::string jsCode{std::istreambuf_iterator<char>(jsFile),
                          std::istreambuf_iterator<char>()};
        jsFile.close();

        json msg = {
            {"id", 2},
            {"method", "Runtime.evaluate"},
            {"params", {
                {"expression", jsCode},
                {"userGesture", true},
                {"awaitPromise", false}
            }}
        };

        std::string msg_text = msg.dump();
        ws.write(boost::asio::buffer(msg_text));

        // Optional: read response
        boost::beast::flat_buffer ws_buffer;
        ws.read(ws_buffer);
        std::cout << boost::beast::make_printable(ws_buffer.data()) << "\n";

        ws.close(websocket::close_code::normal);
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }

    return true;
}
