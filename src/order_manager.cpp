#include "order_manager.hpp"
#include "utils.hpp" // Include your utility functions
#include <iostream>
#include <string>
#include <stdexcept>
#include <rapidjson/document.h>

namespace OrderManagerNamespace {

    void placeOrder(const std::string& instrumentName, double quantity, double price, const std::string& orderType) {
        try {
            std::string accessToken = UtilityNamespace::authenticate();

            std::string url = "https://test.deribit.com/api/v2/private/buy";
            std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/buy\", \"params\":{\"instrument_name\":\"" 
                              + instrumentName + "\", \"amount\":" + std::to_string(quantity) + ", \"price\":" 
                              + std::to_string(price) + ", \"type\":\"" + orderType + "\"}}";
            std::string authHeader = "Authorization: Bearer " + accessToken;

            std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);

            rapidjson::Document jsonResponse;
            jsonResponse.Parse(response.c_str());

            if (jsonResponse.HasMember("result")) {
                UtilityNamespace::logMessage("Order placed successfully for " + instrumentName);
            } else {
                UtilityNamespace::logMessage("Failed to place order. Response: " + response);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error while placing order: " << e.what() << std::endl;
        }
    }

    void getOrderStatus(const std::string& orderId) {
        try {
            std::string accessToken = UtilityNamespace::authenticate();

            std::string url = "https://test.deribit.com/api/v2/private/get_order_state";
            std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/get_order_state\", \"params\":{\"order_id\":\"" 
                                  + orderId + "\"}}";

            std::string authHeader = "Authorization: Bearer " + accessToken;

            std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);
            rapidjson::Document jsonResponse;
            jsonResponse.Parse(response.c_str());

            if (jsonResponse.HasMember("result")) {
                UtilityNamespace::logMessage("Order status: " + std::string(jsonResponse["result"]["order_state"].GetString()));
            } else {
                UtilityNamespace::logMessage("Failed to get order status. Response: " + response);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error while getting order status: " << e.what() << std::endl;
        }
    }

    void cancelOrder(const std::string& orderId) {
        try {
            std::string accessToken = UtilityNamespace::authenticate();

            std::string url = "https://test.deribit.com/api/v2/private/cancel";
            std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/cancel\", \"params\":{\"order_id\":\"" 
                                  + orderId + "\"}}";

            std::string authHeader = "Authorization: Bearer " + accessToken;

            std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);

            rapidjson::Document jsonResponse;
            jsonResponse.Parse(response.c_str());

            if (jsonResponse.HasMember("result")) {
                UtilityNamespace::logMessage("Order canceled successfully: " + orderId);
            } else {
                UtilityNamespace::logMessage("Failed to cancel order. Response: " + response);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error while canceling order: " << e.what() << std::endl;
        }
    }

    void modifyOrder(const std::string& order_id, double amount, double price){
        try {
            std::string accessToken = UtilityNamespace::authenticate();
            std::string url = "https://test.deribit.com/api/v2/private/buy";
            std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/buy\", \"params\":{\"order_id\":\"" 
                              + order_id + "\", \"amount\":" + std::to_string(amount) + ", \"price\":" 
                              + std::to_string(price) + "\"}}";
            std::string authHeader = "Authorization: Bearer " + accessToken;

            std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);

            rapidjson::Document jsonResponse;
            jsonResponse.Parse(response.c_str());

            if (jsonResponse.HasMember("result")) {
                UtilityNamespace::logMessage("Order modiefied successfully for " + order_id);
            } else {
                UtilityNamespace::logMessage("Failed to modify order. Response: " + response);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error while modifying order: " << e.what() << std::endl;
        }
    }
    void getOrderBook(const std::string& symbol) {
        try {
            std::string url = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;
            std::string response = UtilityNamespace::sendGetRequest(url);
            UtilityNamespace::logMessage("Order book for " + symbol + ": " + response);
        } catch (const std::exception& e) {
            std::cerr << "Error while getting order book: " << e.what() << std::endl;
        }
    }
    void getCurrentPositions(const std::string& currency) {
        try {
            std::string accessToken = UtilityNamespace::authenticate();

            std::string url = "https://test.deribit.com/api/v2/private/get_positions";
            std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/get_positions\", "
                                  "\"params\":{\"currency\":\"" + currency + "\"}}";

            std::string authHeader = "Authorization: Bearer " + accessToken;

            std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);

            rapidjson::Document jsonResponse;
            jsonResponse.Parse(response.c_str());

            if (jsonResponse.HasMember("result")) {
                const auto& positions = jsonResponse["result"];

                UtilityNamespace::logMessage("Current positions for " + currency + ":");
                for (const auto& position : positions.GetArray()) {
                    std::string instrument_name = position["instrument_name"].GetString();
                    double size = position["size"].GetDouble();
                    double average_price = position["average_price"].GetDouble();

                    std::cout << "Instrument: " << instrument_name
                              << ", Size: " << size
                              << ", Avg Price: " << average_price << std::endl;
                }
            } else {
                UtilityNamespace::logMessage("Failed to fetch positions. Response: " + response);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error while fetching positions: " << e.what() << std::endl;
        }
    }
}
