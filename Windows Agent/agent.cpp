#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <string>
#include <cstdint>
#include <vector>
#include <tlhelp32.h>


#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "8081"
struct addrinfo *result = NULL, *ptr = NULL, hints;


static float CalculateCPULoad();
static unsigned long long FileTimeToInt64();
float GetCPULoad();
std::string WCHR_UTF8(const wchar_t* wide);

struct ProcessCollection{
    uint64_t timestamp;
    std::string agent_id;
    std::string source;

    int pid;
    int parent_pid;

    std::string process_name; 
    std::string full_path;
    std::string cmd_line;

    std::string process_usr;
};

struct PortCollection{
    uint64_t timestamp;
    std::string agent_id;
    std::string source;

    std::string protocol;
    std::string local_ip;
    int local_port;

    std::string state;

    int owner_pid;
    std::string owning_process;
};

struct ResourceUsageCollection{
    uint64_t timestamp;
    std::string agent_id;
    std::string source;

    float cpu_usage_p;

    uint64_t total_memory_mb;
    uint64_t used_memory_mb;
    uint64_t free_memory_mb;
    double memory_usage_p;

    uint64_t uptime_s;
};


std::vector<ProcessCollection> collectProcesses(){
    std::vector<ProcessCollection> returnVector;
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessSnap ==INVALID_HANDLE_VALUE)
    {
        std::cout << "This will eventually create an err log and exit the program\n";
        return returnVector;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(hProcessSnap, &pe32))
    {
        std::cout << "This will eventually create an err log and exit the program\n";
        CloseHandle(hProcessSnap);          
        return returnVector;
    }
    do
    {
        ProcessCollection proc{};
        proc.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        proc.agent_id = "NOT SET";
        proc.source = "process";
        proc.pid = pe32.th32ProcessID;
        proc.parent_pid = pe32.th32ParentProcessID;
        proc.process_name = WCHR_UTF8(pe32.szExeFile);
        proc.full_path  = "NOT SET";
        proc.cmd_line   = "NOT SET";
        proc.process_usr = "NOT SET";
        returnVector.push_back(proc);
    } while (Process32NextW(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return returnVector;
    
}

// Implement later
/*
std::vector<PortCollection> collectPorts(){
    
}*/

ResourceUsageCollection collectResourceUsage(){
    ResourceUsageCollection instance;
    instance.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    instance.agent_id = "NOT SET";
    instance.source = "resource";
    instance.cpu_usage_p = GetCPULoad() * 100;
    

    instance.total_memory_mb = 0;
    instance.used_memory_mb  = 0;
    instance.free_memory_mb  = 0; // Will implement later
    instance.memory_usage_p  = 0.0;
    instance.uptime_s        = 0;

    return instance;
}


void sendCollectionInfo(){

}


int EventListener(){
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
    printf("Error at socket(): %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return 1;
    }
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
    printf( "Listen failed with error: %ld\n", WSAGetLastError() );
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
    }
    std::cout << "listening on port " << DEFAULT_PORT << std::endl;

while (true) {
    SOCKET ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        continue;
    }
    char recvbuf[512];
    int bytes = recv(ClientSocket, recvbuf, sizeof(recvbuf) - 1, 0);
    if (bytes > 0) {
        recvbuf[bytes] = '\0';
        std::string cmd(recvbuf);
        cmd.erase(cmd.find_last_not_of("\r\n") + 1);
        std::cout << "[CMD RECEIVED] " << cmd << std::endl;
        if (cmd == "COLLECT_PROCESS") {
            auto procs = collectProcesses();
            std::string reply =
                "PROCESS_COUNT=" + std::to_string(procs.size()) + "\n";
            send(ClientSocket, reply.c_str(), (int)reply.size(), 0);
        }
        else if (cmd == "COLLECT_RESOURCE") {
            auto res = collectResourceUsage();
            std::string reply =
                "CPU_USAGE=" + std::to_string(res.cpu_usage_p) + "\n";
            send(ClientSocket, reply.c_str(), (int)reply.size(), 0);
        }
        else {
            std::string reply = "UNKNOWN_COMMAND\n";
            send(ClientSocket, reply.c_str(), (int)reply.size(), 0);
        }
    }
    closesocket(ClientSocket);
}
}


int main() {
    std::thread listenForevents(EventListener);
    listenForevents.join();
    // Main will do more soon
    return 0;
}



std::string WCHR_UTF8(const wchar_t* wide) {
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    std::string str(sizeNeeded - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &str[0], sizeNeeded, nullptr, nullptr);
    return str;
}

static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
    static unsigned long long _previousTotalTicks = 0;
    static unsigned long long _previousIdleTicks = 0;
    unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
    unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;
    float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);
    _previousTotalTicks = totalTicks;
    _previousIdleTicks = idleTicks;
    return ret;
}
static unsigned long long FileTimeToInt64(const FILETIME & ft)
{
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}
float GetCPULoad()
{
    FILETIME idleTime, kernelTime, userTime;
    return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
}


// g++ agent.cpp -o agent.exe -lws2_32 -liphlpapi


// You can currently test with:
// nc 127.0.0.1 8081
// Then typing one of the commands from line 201-214~