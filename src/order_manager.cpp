#include "order_manager.hpp"
#include "utils.hpp" 
#include <iostream>
#include <string>
#include <stdexcept>
#include <rapidjson/document.h>



std::string OrderManager::placeOrder(const std::string& instrumentName, double quantity, double price, const std::string& orderType) {
    try 
    {
        std::string accessToken = UtilityNamespace::authenticate();
        std::string url = "https://test.deribit.com/api/v2/private/buy";
        std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/buy\", \"params\":{\"instrument_name\":\"" 
                          + instrumentName + "\", \"amount\":" + std::to_string(quantity) + ", \"price\":" 
                          + std::to_string(price) + ", \"type\":\"" + orderType + "\"}}";
        std::string authHeader = "Authorization: Bearer " + accessToken;
        std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);
        return response;
    } 
    catch (const std::exception& e) 
    {
        return "Error while placing order: " + std::string(e.what());
    }
}

std::string OrderManager::cancelOrder(const std::string& orderId) {
    try 
    {
        std::string accessToken = UtilityNamespace::authenticate();
        std::string url = "https://test.deribit.com/api/v2/private/cancel";
        std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/cancel\", \"params\":{\"order_id\":\"" 
                              + orderId + "\"}}";
        std::string authHeader = "Authorization: Bearer " + accessToken;
        std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);
        return response;
    } 
    catch (const std::exception& e) 
    {
        return "Error while canceling order: " + std::string(e.what());
    }
}

std::string OrderManager::modifyOrder(const std::string& order_id, double amount, double price) {
    try 
    {
        std::string accessToken = UtilityNamespace::authenticate();
        std::string url = "https://test.deribit.com/api/v2/private/edit";
        std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/edit\", \"params\":{\"order_id\":\"" 
                      + order_id + "\", \"amount\":" + std::to_string(amount) + ", \"price\":" 
                      + std::to_string(price) + "}}"; 
        std::string authHeader = "Authorization: Bearer " + accessToken;
        std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);
        return response;
    } 
    catch (const std::exception& e) 
    {
        return "Error while modifying order: " + std::string(e.what());
    }
}

std::string OrderManager::getOrderBook(const std::string& symbol) {
    try 
    {
        std::string url = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + symbol;
        std::string response = UtilityNamespace::sendGetRequest(url);
        return response;
    } 
    catch (const std::exception& e) 
    {
        return "Error while getting order book: " + std::string(e.what());
    }
}

std::string OrderManager::getCurrentPositions(const std::string& currency) {
    try 
    {
        std::string accessToken = UtilityNamespace::authenticate();
        std::string url = "https://test.deribit.com/api/v2/private/get_positions";
        std::string payload = "{\"jsonrpc\":\"2.0\", \"id\":1, \"method\":\"private/get_positions\", "
                              "\"params\":{\"currency\":\"" + currency + "\"}}";
        std::string authHeader = "Authorization: Bearer " + accessToken;
        std::string response = UtilityNamespace::sendPostRequestWithAuth(url, payload, authHeader);
        return response;
    } 
    catch (const std::exception& e) 
    {
        return "Error while fetching positions: " + std::string(e.what());
    }
}

std::string OrderManager::getInstruments() {
    try 
    {
        std::string url = "https://test.deribit.com/api/v2/public/get_instruments";
        return UtilityNamespace::sendGetRequest(url); 
    } 
    catch(const std::exception& e) 
    {
        return "Error while fetching instruments: " + std::string(e.what());
    }
}

std::string OrderManager::getInstrumentOrderbook(const std::string& instrumentName) {
    try 
    {
        std::string url = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + instrumentName;
        return UtilityNamespace::sendGetRequest(url); 
    } 
    catch(const std::exception& e) 
    {
        return "Error while fetching instrument order book: " + std::string(e.what());
    }
    
}
