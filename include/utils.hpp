#pragma once

#include <string>

namespace UtilityNamespace {

    // Function to authenticate and return an access token
    std::string authenticate();

    // Function to send an HTTP POST request with authorization
    std::string sendPostRequestWithAuth(const std::string& url, const std::string& payload, const std::string& authHeader);

    // Function to send an HTTP POST request
    std::string sendPostRequest(const std::string& url, const std::string& payload);

    // Function to send an HTTP GET request
    std::string sendGetRequest(const std::string& url);

    // Function to get the order book for a specific symbol
    std::string getOrderBook(const std::string& symbol);

    // Function to get a list of instruments
    std::string getInstruments();

    // Function to get the order book for a specific instrument
    std::string getInstrumentOrderbook(const std::string& instrumentName);

    // Function to log messages to the console
    void logMessage(const std::string& message);
}

