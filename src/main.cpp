#include <iostream>
#include "explain.hpp"
#include "config.hpp"

namespace program {

    namespace websocket = beast::websocket;
    using tcp = net::ip::tcp;

    using namespace std::literals;

    int
    run()
    {
        auto host = "cpclientapi.softphone.com"s, endpoint = "/counterpath/socketapi/v1"s, port = "9002"s;
        net::io_context ioc;
        ssl::context ctx{ssl::context::tls_client};
        websocket::stream<ssl::stream<tcp::socket>> m_websocket{ioc, ctx};

        tcp::resolver resolver{ioc};

        const auto resolved = resolver.resolve(host, port);

        boost::asio::connect(m_websocket.next_layer().next_layer(), resolved.begin(), resolved.end());

        m_websocket.next_layer().handshake(ssl::stream_base::client);
        m_websocket.handshake(host, endpoint);

        std::string request = "GET/bringToFront\n"
                              "User-Agent: TestApp\n"
                              "Transaction-ID: AE26f998027\n"
                              "Content-Type: application/xml\n"
                              "Content-Length: 0";
        m_websocket.write(boost::asio::buffer(request));

        beast::flat_buffer m_resBuffer;
        m_websocket.read(m_resBuffer);

        std::cout << beast::buffers_to_string(m_resBuffer.data()) << std::endl;

        m_websocket.close(websocket::close_code::normal);

        return 0;
    }
}

int
main()
{
    try
    {
        return program::run();
    }
    catch (...)
    {
        std::cerr << program::explain() << std::endl;
        return 127;
    }
}