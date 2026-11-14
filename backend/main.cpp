#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <cstring>
#include "SudokuSolver.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int SOCKET;
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define closesocket close
#endif

using namespace std;

void runCliDemo() {
    int sudokuGrid[9][9] = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };

    sudokusolver solver(sudokuGrid);
    cout << "Input Grid:" << endl;
    solver.printGrid();

    if (solver.solve()) {
        cout << "Solved Puzzle:" << endl;
        solver.printGrid();
    } else {
        cout << "Cannot be solved" << endl;
    }
}

bool parseGridFromJson(const string& body, int grid[9][9]) {
    // Parse 81 numbers for 9x9 grid
    size_t pos = 0;
    size_t gridKey = body.find("\"grid\"");
    if (gridKey != string::npos) {
        pos = gridKey + 6;
    }

    int count = 0;
    while (pos < body.size() && count < 81) {
        if (isdigit(body[pos])) {
            int val = 0;
            while (pos < body.size() && isdigit(body[pos])) {
                val = val * 10 + (body[pos] - '0');
                pos++;
            }
            if (val < 0 || val > 9) {
                return false;
            }
            grid[count / 9][count % 9] = val;
            count++;
        } else {
            pos++;
        }
    }
    return count == 81;
}

string makeHttpResponse(int statusCode, const string& statusText, const string& contentType, const string& body) {
    ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    return oss.str();
}

string makeOptionsResponse() {
    ostringstream oss;
    oss << "HTTP/1.1 204 No Content\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type\r\n";
    oss << "Content-Length: 0\r\n";
    oss << "Connection: close\r\n\r\n";
    return oss.str();
}

string formatSolutionJson(int grid[9][9]) {
    ostringstream oss;
    oss << "{\"success\":true,\"solution\":[";
    for (int r = 0; r < 9; ++r) {
        oss << "[";
        for (int c = 0; c < 9; ++c) {
            oss << grid[r][c];
            if (c < 8) oss << ",";
        }
        oss << "]";
        if (r < 8) oss << ",";
    }
    oss << "]}";
    return oss.str();
}

int startServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock" << endl;
        return 1;
    }
#endif

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        cerr << "Failed to create socket" << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed on port " << port << endl;
        closesocket(serverSock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (listen(serverSock, 10) == SOCKET_ERROR) {
        cerr << "Listen failed" << endl;
        closesocket(serverSock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    cout << "Sudoku Solver backend listening on http://localhost:" << port << endl;
    cout << "Ready for frontend requests..." << endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientLen);
        if (clientSock == INVALID_SOCKET) {
            continue;
        }

        string requestData;
        char buffer[4096];
        int bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            requestData.append(buffer, bytesRead);

            // Read remaining body bytes if needed
            size_t headerEnd = requestData.find("\r\n\r\n");
            if (headerEnd != string::npos) {
                size_t clPos = requestData.find("Content-Length:");
                if (clPos == string::npos) {
                    clPos = requestData.find("content-length:");
                }
                if (clPos != string::npos) {
                    size_t lineEnd = requestData.find("\r\n", clPos);
                    string lenStr = requestData.substr(clPos + 15, lineEnd - (clPos + 15));
                    int expectedBodyLen = atoi(lenStr.c_str());
                    size_t currentBodyLen = requestData.size() - (headerEnd + 4);
                    while ((int)currentBodyLen < expectedBodyLen) {
                        int extra = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
                        if (extra <= 0) break;
                        requestData.append(buffer, extra);
                        currentBodyLen += extra;
                    }
                }
            }
        }

        if (requestData.empty()) {
            closesocket(clientSock);
            continue;
        }

        // Parse request method and path
        istringstream reqStream(requestData);
        string method, path;
        reqStream >> method >> path;

        string response;

        if (method == "OPTIONS") {
            response = makeOptionsResponse();
        } else if (method == "GET" && (path == "/" || path == "/health" || path == "/api/health")) {
            string body = "{\"status\":\"ok\",\"message\":\"Sudoku Solver backend is running\"}";
            response = makeHttpResponse(200, "OK", "application/json", body);
        } else if (method == "POST" && (path == "/solve" || path == "/api/solve")) {
            size_t bodyPos = requestData.find("\r\n\r\n");
            string body = (bodyPos != string::npos) ? requestData.substr(bodyPos + 4) : "";

            int inputGrid[9][9] = {0};
            if (!parseGridFromJson(body, inputGrid)) {
                string err = "{\"success\":false,\"error\":\"Invalid grid format. 81 cell values (0-9) required.\"}";
                response = makeHttpResponse(400, "Bad Request", "application/json", err);
            } else {
                sudokusolver solver(inputGrid);
                if (solver.solve()) {
                    int solvedGrid[9][9];
                    solver.getGrid(solvedGrid);
                    string jsonBody = formatSolutionJson(solvedGrid);
                    response = makeHttpResponse(200, "OK", "application/json", jsonBody);
                } else {
                    string err = "{\"success\":false,\"error\":\"This puzzle has no valid solution.\"}";
                    response = makeHttpResponse(200, "OK", "application/json", err);
                }
            }
        } else {
            string notFound = "{\"error\":\"Not Found\"}";
            response = makeHttpResponse(404, "Not Found", "application/json", notFound);
        }

        send(clientSock, response.c_str(), (int)response.size(), 0);
        closesocket(clientSock);
    }

    closesocket(serverSock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

void runTests() {
    string sampleJson = "{\"grid\":[[5,3,0,0,7,0,0,0,0],[6,0,0,1,9,5,0,0,0],[0,9,8,0,0,0,0,6,0],[8,0,0,0,6,0,0,0,3],[4,0,0,8,0,3,0,0,1],[7,0,0,0,2,0,0,0,6],[0,6,0,0,0,0,2,8,0],[0,0,0,4,1,9,0,0,5],[0,0,0,0,8,0,0,7,9]]}";
    int grid[9][9] = {0};
    if (!parseGridFromJson(sampleJson, grid)) {
        cout << "TEST FAILED: parseGridFromJson failed" << endl;
        return;
    }
    cout << "TEST PASSED: parseGridFromJson successfully parsed 81 cells" << endl;

    sudokusolver solver(grid);
    if (!solver.solve()) {
        cout << "TEST FAILED: solver.solve returned false for valid puzzle" << endl;
        return;
    }
    int solved[9][9];
    solver.getGrid(solved);
    string formatted = formatSolutionJson(solved);
    if (formatted.find("\"success\":true") == string::npos) {
        cout << "TEST FAILED: formatSolutionJson missing success field" << endl;
        return;
    }
    cout << "TEST PASSED: solver.solve and formatSolutionJson succeeded" << endl;

    // Test invalid initial board rejection
    int invalidGrid[9][9] = {0};
    invalidGrid[0][0] = 5;
    invalidGrid[0][1] = 5;
    sudokusolver invalidSolver(invalidGrid);
    if (invalidSolver.solve()) {
        cout << "TEST FAILED: solver solved an invalid board with duplicates" << endl;
        return;
    }
    cout << "TEST PASSED: solver rejected invalid initial board" << endl;
    cout << "All internal tests passed." << endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "--test") {
        runTests();
        return 0;
    }

    if (argc > 1 && string(argv[1]) == "--cli") {
        runCliDemo();
        return 0;
    }

    int port = 5000;
    if (argc > 1) {
        int parsedPort = atoi(argv[1]);
        if (parsedPort > 0) {
            port = parsedPort;
        }
    }

    return startServer(port);
}
