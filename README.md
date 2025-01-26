# Order Execution and Management System

## Overview
This project is a high-performance Order Execution and Management System (OEMS) designed for trading platforms. It provides a set of APIs to place, modify, cancel orders, and retrieve order book data, along with support for real-time market data streaming via WebSocket. The system is built to interface with the Deribit exchange, supporting spot, futures, and options trading.

## Features

### Order Management Functions
1. **Place order**: API to place a new order.
2. **Cancel order**: API to cancel an existing order.
3. **Modify order**: API to modify an existing order.
4. **Get orderbook**: Fetch the current order book for a given symbol.
5. **View current positions**: Retrieve information about the user's current positions.
6. **Real-time market data streaming via WebSocket**:
   - Implement WebSocket server functionality.
   - Allow clients to subscribe to symbols.
   - Stream continuous orderbook updates for subscribed symbols.

### Market Coverage
- **Instruments**: Spot, Futures, and Options.
- **Scope**: All supported symbols on Deribit.

## Getting Started

To get started with this project, follow the steps below:
1. **Create an account on Deribit Test Network**
    - Go to the [Deribit Test Network](https://test.deribit.com/) website.
    - Sign up for a new account or log in if you already have one.
    - Once logged in, navigate to the **API Keys** section in your account settings.
    - Generate a new API key with the necessary permissions (e.g., access to place orders, view positions.
    - Save your **API key** and **Secret** for later use.

2. **Clone the repository**:
   ```bash
   git clone https://github.com/your-username/OEMS_GoQuant.git
   cd OEMS_GoQuant
3. **Make a Config.hpp file in Include folder and save your API Keys there**
    ```bash 
    #pragma once
    #include <string>
    const std::string API_KEY = "YOUR_API_KEY";   
    const std::string SECRET_KEY = "YOUR_SECRET"; 
5. **Create a build folder in the root directory**:
   ```bash
   mkdir build
   cd build
6. **Run the following CMake commands**:
   ```bash
   cmake ..
   cmake --build . --config Debug
7. **Navigate to the bin/Debug folder and run the executable**:
   ```bash
   cd bin/Debug
   ./deribit_order_management.exe

## Troubleshooting

If you encounter any issues during the build or execution, follow these steps to resolve common problems:

1. **Dependencies**: 
   - Ensure that all dependencies are correctly installed and up-to-date.
   - Verify that your environment is properly set up for building C++ projects with CMake.

2. **Network Connectivity or API Credentials**: 
   - Check the `deribit_order_management.exe` output for any errors related to network connectivity or API credentials.
   - Make sure your API credentials are valid and that you have internet access.

3. **Real-time WebSocket Streaming**:
   - Ensure that you have a stable internet connection, especially for real-time WebSocket streaming.
   - Check if the WebSocket server is running correctly and that you are subscribed to the correct symbols.

If the issue persists, please refer to the logs or open an issue on the repository.
