#pragma once

#include <string>

namespace UtilityNamespace {

    std::string authenticate();
    std::string sendPostRequestWithAuth(const std::string& url, const std::string& payload, const std::string& authHeader);
    std::string sendPostRequest(const std::string& url, const std::string& payload);
    std::string sendGetRequest(const std::string& url);
    std::string getOrderBook(const std::string& symbol);
    std::string getInstruments();
    std::string getInstrumentOrderbook(const std::string& instrumentName);
    void logMessage(const std::string& message);
}

