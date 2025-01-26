#include "websocket_handler.hpp"
#include "utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    s->append((char*)contents, newLength);
    return newLength;
}

WebSocketHandler::WebSocketHandler() : m_running(false) {
    m_server.init_asio();

    m_server.set_message_handler([this](connection_hdl hdl, server::message_ptr msg) {
        handleMessage(hdl, msg);
    });

    m_server.set_open_handler([this](connection_hdl hdl) {
        std::cout << "New client connected.\n";
    });

    m_server.set_close_handler([this](connection_hdl hdl) {
        handleClose(hdl);
    });
}

WebSocketHandler::~WebSocketHandler() {
    stopServer(); 
}

void WebSocketHandler::stopServer() {
    if (!m_running.exchange(false)) {
        std::cerr << "Server is not running.\n";
        return;
    }

    m_server.stop_listening(); 
    m_server.stop(); 
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }

    m_server.get_io_service().reset();
    std::cout << "Server stopped.\n";
}

void WebSocketHandler::startServer(uint16_t port) {
    if (m_running.exchange(true)) {
        std::cerr << "Server is already running.\n";
        return;
    }
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    m_server.clear_error_channels(websocketpp::log::elevel::all);

    m_serverThread = std::thread([this, port]() { run(port); });
    std::cout << "Server started on port " << port << "\n";
}

void WebSocketHandler::run(uint16_t port) {
    try {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
    }
}

void WebSocketHandler::handleMessage(connection_hdl hdl, server::message_ptr msg) {
    // On the server side
    auto startTime = std::chrono::high_resolution_clock::now();

    std::string payload = msg->get_payload();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto propagationDelay = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "WebSocket Message Propagation Delay: " << propagationDelay << "ms" << std::endl;
    rapidjson::Document doc;
    doc.Parse(payload.c_str());

    if (doc.HasParseError() || !doc.HasMember("action")) {
        m_server.send(hdl, R"({"error": "Invalid message format"})", websocketpp::frame::opcode::text);
        return;
    }

    std::string action = doc["action"].GetString();

    if (action == "subscribe" && doc.HasMember("symbol")) {
        std::string symbol = doc["symbol"].GetString();
        {
            std::lock_guard<std::mutex> lock(m_subscriptionMutex);
            m_subscriptions[symbol].insert(hdl);
        }
        std::cout << "Client subscribed to: " << symbol << "\n";
        m_server.send(hdl, "Subscribed to " + symbol, websocketpp::frame::opcode::text);
    } else if (action == "unsubscribe" && doc.HasMember("symbol")) {
        std::string symbol = doc["symbol"].GetString();
        {
            std::lock_guard<std::mutex> lock(m_subscriptionMutex);
            if (m_subscriptions.count(symbol)) {
                m_subscriptions[symbol].erase(hdl);
                if (m_subscriptions[symbol].empty()) {
                    m_subscriptions.erase(symbol);
                }
                std::cout << "Client unsubscribed from: " << symbol << "\n";
                m_server.send(hdl, "Unsubscribed from " + symbol, websocketpp::frame::opcode::text);
            } else {
                m_server.send(hdl, "Symbol not found in subscriptions", websocketpp::frame::opcode::text);
            }
        }
    } else {
        m_server.send(hdl, "Unknown command", websocketpp::frame::opcode::text);
    }
}

void WebSocketHandler::handleClose(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_subscriptionMutex);
    for (auto& [symbol, clients] : m_subscriptions) {
        clients.erase(hdl); 
    }
    std::cout << "Client disconnected.\n";
}

std::string WebSocketHandler::getOrderBook(const std::string& symbol) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL Error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        if (headers) {
            curl_slist_free_all(headers);
        }
    }

    if (!readBuffer.empty()) {
        // Return the fetched JSON response
        return readBuffer;
    } else {
        // Return an error response in case of failure
        return R"({"error": "Failed to fetch order book"})";
    }
}

void WebSocketHandler::broadcastOrderBookUpdates(std::atomic<bool>& isBroadcasting) {
    while (m_running && isBroadcasting) {
        std::lock_guard<std::mutex> lock(m_subscriptionMutex);
        for (const auto& [symbol, clients] : m_subscriptions) {
            std::string orderBookData = getOrderBook(symbol);
            for (const auto& client : clients) {
                try {
                    m_server.send(client, orderBookData, websocketpp::frame::opcode::text);
                } catch (const websocketpp::exception& e) {
                    std::cerr << "Error sending to client: " << e.what() << std::endl;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
    }
}