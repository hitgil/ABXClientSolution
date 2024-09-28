# ABXClientSolution
C++ client application for interacting with the ABX mock exchange server. The client connects via TCP, requests stock ticker data, and generates a JSON file with complete, ordered data sequences.

## Dependencies

- C++11 or later
- [nlohmann/json](https://github.com/nlohmann/json): A JSON library for modern C++ (Install via Homebrew: `brew install nlohmann-json`)


## Installation

1. **Clone the repository**:

   ```bash
   git clone <repository-url>
   cd <repository-name>

2. **Install dependencies**:
    ```bash
    Install dependencies
    ```
3. **COmpile the application**:

    ```bash
    g++ ./src/*.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include -std=c++11 -o output
    ```
4. **Run the application**:

    ```bash
    ./output
    ```
5. Example Output

    ``` Connected to server  
    Data sent to server.  
    Data received from server: 187 bytes  
    Requesting missing packet with sequence: 1  
    Received missing packet for sequence: 1  
    Requesting missing packet with sequence: 6  
    Received missing packet for sequence: 6  
    Requesting missing packet with sequence: 8  
    Received missing packet for sequence: 8  
    JSON file created: packets.json
    ```


