#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdint>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>


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
        proc.agent_id = "null"; // I'll add later
        proc.source = "process";
        proc.pid = pe32.th32ProcessID;
        proc.parent_pid = pe32.th32ParentProcessID;
        proc.process_name = WCHR_UTF8(pe32.szExeFile);
        proc.full_path  = ""; // I'll add later
        proc.cmd_line   = ""; // I'll add later
        proc.process_usr = ""; // I'll add later
        returnVector.push_back(proc);
    } while (Process32NextW(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return returnVector;
    
}
/*
std::vector<PortCollection> collectPorts(){
    
}*/

ResourceUsageCollection collectResourceUsage(){
    ResourceUsageCollection instance;
    instance.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    instance.agent_id = "null"; // Will implement later
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

void listener(){

}




int main() {

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