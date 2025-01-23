#pragma once
#include <iostream>
#include "order_manager.hpp"
#include "utils.hpp"
#include <rapidjson/document.h>

int main() {
    std::cout << "Program Started!" << std::endl;
    try {
        std::cout << "Authenticating..." << std::endl;
        std::string accessToken = UtilityNamespace::authenticate(); 
        if (accessToken.empty()) {
            throw std::runtime_error("Authentication failed. Access token is empty.");
        }
        UtilityNamespace::logMessage("Successfully authenticated. Access token acquired.");

        std::cout << "Placing an order..." << std::endl;
        std::string instrumentName = "BTC-PERPETUAL"; 
        double quantity = 10.0;                      
        double price = 25000.0;                       
        std::string orderType = "limit";              

        OrderManager orderManager;
        std::string orderResponse = orderManager.placeOrder(instrumentName, quantity, price, orderType);
        // if (orderResponse.empty()) {
        //     throw std::runtime_error("Failed to place the order. Response is empty.");
        // }
        rapidjson::Document jsonResponse;
        jsonResponse.Parse(orderResponse.c_str());
        std::string orderId = jsonResponse["result"]["order"]["order_id"].GetString();
        UtilityNamespace::logMessage("Order placed successfully: " + orderId);
        std::cout << "Order Response: " << orderId << std::endl;

        std::cout << "Modifying the order..." << std::endl;
        double newQuantity = 10.0;  
        double newPrice = 24000.0;  
        std::string modifyResponse = orderManager.modifyOrder(orderId, newQuantity, newPrice);
        // if (modifyResponse.empty()) {
        //     throw std::runtime_error("Failed to modify the order.");
        // }
        UtilityNamespace::logMessage("Order modified successfully: " + modifyResponse);
        std::cout << "Modify Response: " << modifyResponse << std::endl;

        // std::cout << "Cancelling the order..." << std::endl;
        // std::string cancelId = orderId;
        // std::string cancelResponse = orderManager.cancelOrder(cancelId);
        // // if (cancelResponse.empty()) {
        // //     throw std::runtime_error("Failed to cancel the order.");
        // // }
        // UtilityNamespace::logMessage("Order cancelled successfully: " + cancelResponse);
        // std::cout << "Cancel Response: " << cancelResponse << std::endl;

        // std::cout << "Fetching current positions..." << std::endl;
        // std::string currency = "BTC"; 
        // std::string positions = orderManager.getCurrentPositions(currency);
        // if (positions.empty()) {
        //     UtilityNamespace::logMessage("No positions found for currency: " + currency);
        // } else {
        //     UtilityNamespace::logMessage("Current positions fetched successfully.");
        //     std::cout << "Positions for " << currency << ":\n" << positions << std::endl;
        // }

        // std::cout << "Fetching order book..." << std::endl;
        // std::string orderBook = UtilityNamespace::getOrderBook(instrumentName);
        // if (orderBook.empty()) {
        //     UtilityNamespace::logMessage("Order book for " + instrumentName + " is empty.");
        // } else {
        //     UtilityNamespace::logMessage("Order book fetched successfully.");
        //     std::cout << "Order Book for " << instrumentName << ":\n" << orderBook << std::endl;
        // }

        // std::cout << "Fetching all instruments..." << std::endl;
        // std::string instruments = UtilityNamespace::getInstruments();
        // if (instruments.empty()) {
        //     UtilityNamespace::logMessage("No instruments found.");
        // } else {
        //     UtilityNamespace::logMessage("Instruments fetched successfully.");
        //     std::cout << "Available Instruments:\n" << instruments << std::endl;
        // }

    } catch (const std::exception& e) {
        UtilityNamespace::logMessage("An error occurred: " + std::string(e.what()));
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
    // std::cout << "Hello World!" << std::endl;
    return 0;
}
