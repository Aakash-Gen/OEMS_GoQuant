#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include "rapidjson/document.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include "utils.hpp"
#include "order_manager.hpp"


void prettyPrintJSON(const std::string& rawJson) {
    rapidjson::Document document;
    document.Parse(rawJson.c_str());

    if (document.HasParseError()) {
        std::cout << "Invalid JSON: " << rawJson << std::endl;
        return;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::cout << buffer.GetString() << std::endl;
}

void placeOrder(OrderManager& orderManager) {
    std::string instrumentName;
    double quantity, price;
    std::string orderType;

    std::cout << "Enter instrument name: ";
    std::cin >> instrumentName;
    std::cout << "Enter quantity: ";
    std::cin >> quantity;
    std::cout << "Enter price: ";
    std::cin >> price;
    std::cout << "Enter order type (limit/market): ";
    std::cin >> orderType;

    std::string orderResponse = orderManager.placeOrder(instrumentName, quantity, price, orderType);
    rapidjson::Document jsonResponse;
    jsonResponse.Parse(orderResponse.c_str());
    if (jsonResponse.HasParseError() || !jsonResponse.HasMember("result") || !jsonResponse["result"].HasMember("order")) {
        throw std::runtime_error("Invalid response received while placing order.");
    }
    std::string orderId = jsonResponse["result"]["order"]["order_id"].GetString();
    UtilityNamespace::logMessage("Order placed successfully: " + orderId);
    std::cout << "Order Id: " << orderId << std::endl;
}

void modifyOrder(OrderManager& orderManager) {
    std::string orderId;
    double newQuantity, newPrice;

    std::cout << "Enter order ID to modify: ";
    std::cin >> orderId;
    std::cout << "Enter new quantity: ";
    std::cin >> newQuantity;
    std::cout << "Enter new price: ";
    std::cin >> newPrice;

    std::string modifyResponse = orderManager.modifyOrder(orderId, newQuantity, newPrice);

    prettyPrintJSON(modifyResponse);
    UtilityNamespace::logMessage("Order modified successfully: " + modifyResponse);
    // std::cout << "Modify Response: " << modifyResponse << std::endl;
}

void cancelOrder(OrderManager& orderManager) {
    std::string orderId;

    std::cout << "Enter order ID to cancel: ";
    std::cin >> orderId;

    std::string cancelResponse = orderManager.cancelOrder(orderId);
    UtilityNamespace::logMessage("Order cancelled successfully: " + cancelResponse);
    std::cout << "Cancel Response: " << cancelResponse << std::endl;
}

void fetchCurrentPositions(OrderManager& orderManager) {
    std::string currency;
    std::cout << "Enter currency to fetch positions (e.g., BTC): ";
    std::cin >> currency;

    std::cout << "Fetching current positions..." << std::endl;
    std::string response = orderManager.getCurrentPositions(currency);

    // Pretty-print the raw JSON response
    std::cout << "Raw JSON Response (Pretty):" << std::endl;
    prettyPrintJSON(response);

    rapidjson::Document jsonResponse;
    jsonResponse.Parse(response.c_str());

    if (jsonResponse.HasParseError()) {
        std::cout << "Failed to parse response. Response: " << response << std::endl;
        return;
    }

    if (jsonResponse.HasMember("result") && jsonResponse["result"].IsArray()) {
        const auto& positions = jsonResponse["result"].GetArray();
        if (positions.Empty()) {
            std::cout << "No active positions for " << currency << "." << std::endl;
            return;
        }

        // Print header
        std::cout << std::left << std::setw(20) << "Instrument"
                  << std::setw(10) << "Size"
                  << std::setw(15) << "Avg Price"
                  << std::setw(15) << "Floating P&L"
                  << std::setw(15) << "Realized P&L"
                  << std::setw(10) << "Leverage" << std::endl;
        std::cout << std::string(85, '-') << std::endl;

        // Print positions
        for (const auto& position : positions) {
            if (position.HasMember("instrument_name") && position.HasMember("size") &&
                position.HasMember("average_price") && position.HasMember("floating_profit_loss") &&
                position.HasMember("realized_profit_loss") && position.HasMember("leverage")) {
                
                std::string instrument_name = position["instrument_name"].GetString();
                double size = position["size"].GetDouble();
                double average_price = position["average_price"].GetDouble();
                double floating_pnl = position["floating_profit_loss"].GetDouble();
                double realized_pnl = position["realized_profit_loss"].GetDouble();
                double leverage = position["leverage"].GetDouble();

                std::cout << std::left << std::setw(20) << instrument_name
                          << std::setw(10) << size
                          << std::setw(15) << average_price
                          << std::setw(15) << floating_pnl
                          << std::setw(15) << realized_pnl
                          << std::setw(10) << leverage << std::endl;
            } else {
                std::cout << "Position data is incomplete or invalid." << std::endl;
            }
        }
    } else {
        std::cout << "No positions found or invalid response. Response: " << response << std::endl;
    }
}

void fetchOrderBook(OrderManager& orderManager) {
    std::string instrumentName;
    std::cout << "Enter instrument name to fetch order book (e.g., BTC-PERPETUAL): ";
    std::cin >> instrumentName;

    std::cout << "Fetching order book..." << std::endl;
    std::string orderBook = orderManager.getOrderBook(instrumentName);

    if (orderBook.empty()) {
        UtilityNamespace::logMessage("Order book for " + instrumentName + " is empty.");
        std::cout << "Order book for " << instrumentName << " is empty." << std::endl;
        return;
    }

    rapidjson::Document jsonResponse;
    jsonResponse.Parse(orderBook.c_str());

    if (jsonResponse.HasParseError() || 
        !jsonResponse.HasMember("result") || 
        !jsonResponse["result"].IsObject() || 
        !jsonResponse["result"].HasMember("bids") || 
        !jsonResponse["result"]["bids"].IsArray() || 
        !jsonResponse["result"].HasMember("asks") || 
        !jsonResponse["result"]["asks"].IsArray()) {
        throw std::runtime_error("Invalid or incomplete order book data.");
    }

    const auto& bids = jsonResponse["result"]["bids"];
    const auto& asks = jsonResponse["result"]["asks"];

    std::cout << "Order Book for " << instrumentName << ":\n";
    std::cout << std::setw(20) << "Bids (Price x Amount)" 
              << " | " 
              << std::setw(20) << "Asks (Price x Amount)" 
              << "\n";
    std::cout << std::string(45, '-') << "\n";

    size_t maxRows = std::max(bids.Size(), asks.Size());
    for (size_t i = 0; i < maxRows; ++i) {
        std::string bidStr = (i < bids.Size()) 
            ? (std::to_string(bids[i][0].GetDouble()) + " x " + std::to_string(bids[i][1].GetDouble())) 
            : "";

        std::string askStr = (i < asks.Size()) 
            ? (std::to_string(asks[i][0].GetDouble()) + " x " + std::to_string(asks[i][1].GetDouble())) 
            : "";

        std::cout << std::setw(20) << bidStr 
                  << " | " 
                  << std::setw(20) << askStr 
                  << "\n";
    }

    UtilityNamespace::logMessage("Order book fetched successfully.");
}



int main() {
    std::cout << "Program Started!" << std::endl;
    try {
        std::cout << "Authenticating..." << std::endl;
        std::string accessToken = UtilityNamespace::authenticate();
        if (accessToken.empty()) {
            throw std::runtime_error("Authentication failed. Access token is empty.");
        }
        UtilityNamespace::logMessage("Successfully authenticated. Access token acquired.");

        OrderManager orderManager;

        while (true) {
            std::cout << "\nChoose an action:\n";
            std::cout << "1. Place Order\n";
            std::cout << "2. Modify Order\n";
            std::cout << "3. Cancel Order\n";
            std::cout << "4. Fetch Current Positions\n";
            std::cout << "5. Fetch Order Book\n";
            std::cout << "6. Fetch Instruments\n";
            std::cout << "7. Fetch Instrument Order Book\n";
            std::cout << "8. Exit\n";
            std::cout << "Enter your choice: ";

            int choice;
            std::cin >> choice;

            switch (choice) {
                case 1:
                    placeOrder(orderManager);
                    break;
                case 2:
                    modifyOrder(orderManager);
                    break;
                case 3:
                    cancelOrder(orderManager);
                    break;
                case 4:
                    fetchCurrentPositions(orderManager);
                    break;
                case 5:
                    fetchOrderBook(orderManager);
                    break;
                case 6:
                    std::cout << "Exiting program." << std::endl;
                    return 0;
                default:
                    std::cout << "Invalid choice. Please try again." << std::endl;
                    break;
            }
        }

    } catch (const std::exception& e) {
        UtilityNamespace::logMessage("An error occurred: " + std::string(e.what()));
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}
