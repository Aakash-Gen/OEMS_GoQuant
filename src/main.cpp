#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <string>
#include <limits>
#include <stdexcept>
#include "rapidjson/document.h"
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include "utils.hpp"
#include "order_manager.hpp"
#include "websocket_handler.hpp"

void websocketServerControl(WebSocketHandler& wsHandler, std::atomic<bool>& isRunning, std::atomic<bool>& isBroadcasting) {
    std::cout << "\nWebSocket Server Control Commands:\n";
    std::cout << " - start <port>: Start the WebSocket server on the specified port\n";
    std::cout << " - stop: Stop the WebSocket server\n";
    std::cout << " - broadcast: Start broadcasting order book updates\n";
    std::cout << " - stop_broadcast: Stop broadcasting updates\n";
    std::cout << " - back: Return to the main menu\n";

    std::string command;
    while (true) {
        std::cout << "\nEnter WebSocket command: ";
        std::cin >> command;

        if (command == "start") {
            int port;
            std::cin >> port;
            if (isRunning) {
                std::cout << "WebSocket server is already running.\n";
            } else {
                wsHandler.startServer(port);
                isRunning = true;
                std::cout << "WebSocket server started on port " << port << ".\n";
            }
        } else if (command == "stop") {
            if (!isRunning) {
                std::cout << "WebSocket server is not running.\n";
            } else {
                wsHandler.stopServer();
                isRunning = false;
                std::cout << "WebSocket server stopped.\n";
            }
        } else if (command == "broadcast") {
            if (isBroadcasting) {
                std::cout << "Broadcasting is already active.\n";
            } else {
                isBroadcasting = true;
                std::thread broadcasterThread([&]() {
                    wsHandler.broadcastOrderBookUpdates(isBroadcasting);
                    isBroadcasting = false; 
                });
                broadcasterThread.detach();
                std::cout << "Broadcasting started.\n";
            }
        } else if (command == "stop_broadcast") {
            if (!isBroadcasting) {
                std::cout << "Broadcasting is not active.\n";
            } else {
                isBroadcasting = false;
                std::cout << "Broadcasting has stopped.\n";
            }
        } else if (command == "back") {
            break; 
        } else {
            std::cout << "Invalid command. Please try again.\n";
        }
    }
}

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
    std::string type;

    std::cout << "Enter order type (buy/sell): ";
    std::cin >> type;
    std::cout << "Enter instrument name: ";
    std::cin >> instrumentName;
    std::cout << "Enter quantity: ";
    std::cin >> quantity;
    // std::cout << "Enter price: ";
    // std::cin >> price;
    std::cout << "Enter order type (limit/market): ";
    std::cin >> orderType;
    auto startTime1 = std::chrono::high_resolution_clock::now();
    if (orderType == "market") {
        auto startTime = std::chrono::high_resolution_clock::now();
        std::string orderBookJson = orderManager.getOrderBook(instrumentName);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto marketDataProcessingLatency = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        std::cout << "Market Data Processing Latency: " << marketDataProcessingLatency << "ms" << std::endl;

        rapidjson::Document orderBook;
        orderBook.Parse(orderBookJson.c_str());

        if (orderBook.HasParseError() || !orderBook["result"].HasMember("bids") || !orderBook["result"].HasMember("asks")) {
            throw std::runtime_error("Failed to fetch order book or invalid order book format.");
        }

        if (type == "buy") {
            price = orderBook["result"]["asks"].GetArray()[0].GetArray()[0].GetDouble();
        } else if (type == "sell") {
            price = orderBook["result"]["bids"].GetArray()[0].GetArray()[0].GetDouble();
        }

        std::cout << "Market order price fetched from order book: " << price << std::endl;
    } else {
        std::cout << "Enter price: ";
        std::cin >> price;
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::string orderResponse = orderManager.placeOrder(instrumentName,type, quantity, price, orderType);
    auto end = std::chrono::high_resolution_clock::now();auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Order Placement Latency: " << latency << " ms\n";
    rapidjson::Document jsonResponse;
    jsonResponse.Parse(orderResponse.c_str());
    if (jsonResponse.HasParseError() || !jsonResponse.HasMember("result") || !jsonResponse["result"].HasMember("order")) {
        throw std::runtime_error("Invalid response received while placing order.");
    }
    std::string orderId = jsonResponse["result"]["order"]["order_id"].GetString();
    UtilityNamespace::logMessage("Order placed successfully: " + orderId);
    std::cout << "Order Id: " << orderId << std::endl;
    auto endTime1 = std::chrono::high_resolution_clock::now();
    auto EndToEnd_Latency = std::chrono::duration_cast<std::chrono::milliseconds>(endTime1 - startTime1).count();
    std::cout << "End to End Trading Loop Latency: " << EndToEnd_Latency << "ms" << std::endl;
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
    UtilityNamespace::logMessage("Order modified successfully");
    // std::cout << "Modify Response: " << modifyResponse << std::endl;
}

void cancelOrder(OrderManager& orderManager) {
    std::string orderId;

    std::cout << "Enter order ID to cancel: ";
    std::cin >> orderId;

    std::string cancelResponse = orderManager.cancelOrder(orderId);
    UtilityNamespace::logMessage("Order cancelled successfully");
    std::cout << "Cancel Response: " << std::endl;
    prettyPrintJSON(cancelResponse);
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
    auto startTime = std::chrono::high_resolution_clock::now();
    std::string orderBook = orderManager.getOrderBook(instrumentName);
    // auto endTime = std::chrono::high_resolution_clock::now();
    // auto marketDataProcessingLatency = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    // std::cout << "Market Data Processing Latency: " << marketDataProcessingLatency << "ms" << std::endl;
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
    auto endTime = std::chrono::high_resolution_clock::now();
    auto marketDataProcessingLatency = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Market Data Processing Latency: " << marketDataProcessingLatency << "ms" << std::endl;
    UtilityNamespace::logMessage("Order book fetched successfully.");
}

void printInstruments(OrderManager& orderManager) {
    std::string instruments = orderManager.getInstruments();
    // std::cout << "Instruments JSON: " << instruments << std::endl;

    rapidjson::Document document;
    rapidjson::ParseResult parseResult = document.Parse(instruments.c_str());
    if (!parseResult) {
        std::cerr << "JSON parse error: " << rapidjson::GetParseError_En(parseResult.Code())
                  << " at offset " << parseResult.Offset() << std::endl;
        return;
    }
    std::string inputSymbol, inputKind;
    std::cout << "Enter the symbol (e.g., BTC, ETH): ";
    std::cin >> inputSymbol;
    std::cout << "Enter the kind (e.g., future, option, etc.): ";
    std::cin >> inputKind;

    bool found = false;
    std::cout << "Instruments for symbol " << inputSymbol << " of kind " << inputKind << ":" << std::endl;

    if (document.HasMember("result") && document["result"].IsArray()) {
        for (const auto& instrument : document["result"].GetArray()) {
            if (instrument.IsObject() && instrument.HasMember("instrument_name") && instrument.HasMember("kind")) {
                std::string instrumentName = instrument["instrument_name"].GetString();
                std::string kind = instrument["kind"].GetString();

                if (instrumentName.find(inputSymbol) != std::string::npos && kind == inputKind) {
                    std::cout << instrumentName << std::endl;
                    found = true;
                }
            }
        }
    } 
    if(!found) {
        std::cout << "No instruments found for symbol " << inputSymbol << " of kind " << inputKind << std::endl;
    }
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
        WebSocketHandler wsHandler;
        std::atomic<bool> isRunning(false);
        std::atomic<bool> isBroadcasting(false);

        while (true) {
            std::cout << "\nChoose an action:\n";
            std::cout << "1. Place Order\n";
            std::cout << "2. Modify Order\n";
            std::cout << "3. Cancel Order\n";
            std::cout << "4. Fetch Current Positions\n";
            std::cout << "5. Fetch Order Book\n";
            std::cout << "6. Get instruments\n";
            std::cout << "7. WebSocket Server Control\n";
            std::cout << "8. Exit\n";
            std::cout << "Enter your choice: ";

            int choice;
            std::cin >> choice;

            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number between 1 and 8.\n";
                continue;
            }

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
                    printInstruments(orderManager);
                    break;
                case 7:
                    websocketServerControl(wsHandler, isRunning, isBroadcasting);
                    break;
                case 8:
                    if (isRunning) {
                        wsHandler.stopServer();
                    }
                    if (isBroadcasting) {
                        isBroadcasting = false; 
                    }
                    std::cout << "Exiting program." << std::endl;
                    return 0;
                default:
                    std::cout << "Invalid choice. Please enter a number between 1 and 8.\n";
                    break;
            }
        }

    } catch (const std::exception& e) {
        UtilityNamespace::logMessage("An error occurred: " + std::string(e.what()));
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}