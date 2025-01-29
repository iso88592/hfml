#include "server.hpp"
#include "translator.hpp"
using namespace cinatra;

int main() {
    Translator translator;
    int max_thread_num = std::thread::hardware_concurrency();
    coro_http_server server(max_thread_num, 8080);
        server.set_http_handler<OPTIONS>("/", [](coro_http_request& /*ignored*/, coro_http_response& res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_status_and_content(status_type::ok, "");
    });
    server.set_http_handler<GET, POST>("/", [&translator](coro_http_request& req, coro_http_response& res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        if (req.get_method() == "POST") {
            auto body = std::string{req.get_body()};
    
            res.set_status_and_content(status_type::ok, translator.translate(body));
        } else {
            res.set_status_and_content(status_type::ok, "hello world");
        }
    });

    server.sync_start();
    return 0;
}