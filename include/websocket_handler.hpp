#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <unordered_map>
#include <set>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

class WebSocketHandler {
public:
    WebSocketHandler();
    ~WebSocketHandler();

    void startServer(uint16_t port);
    void stopServer();
    void broadcastOrderBookUpdates(std::atomic<bool>& isBroadcasting);

private:
    server m_server;
    std::unordered_map<std::string, std::set<connection_hdl, std::owner_less<connection_hdl>>> m_subscriptions;
    std::mutex m_subscriptionMutex;
    std::thread m_serverThread;
    std::atomic<bool> m_running;

    void run(uint16_t port);
    void handleMessage(connection_hdl hdl, server::message_ptr msg);
    void handleClose(connection_hdl hdl);
    std::string getOrderBook(const std::string& symbol);
};