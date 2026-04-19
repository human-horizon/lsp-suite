#include <iostream>
#include <string>
#include <map>
#include "lsp.h"

static std::string read_request() {
    std::string header;
    size_t content_length = 0;

    while (true) {
        char c;
        if (!std::cin.read(&c, 1)) return "";
        if (c == '\n') {
            if (header.find("Content-Length: ") == 0) {
                content_length = std::stoi(header.substr(15));
            }
            if (header.empty()) break;
            header.clear();
        } else {
            header += c;
        }
    }

    if (content_length == 0) return "";

    std::string body(content_length, '\0');
    std::cin.read(body.data(), content_length);
    return body;
}

static void send_response(const std::string& response) {
    std::cout << "Content-Length: " << response.size() << "\r\n\r\n" << response;
    std::cout.flush();
}

int main() {
    LspServer server;

    while (true) {
        std::string body = read_request();
        if (body.empty()) break;

        auto response = server.handle(body);
        if (!response.empty()) {
            send_response(response);
        }
    }

    return 0;
}
