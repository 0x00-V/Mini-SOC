#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdint>
#include <vector>
#include <Windows.h>


static float CalculateCPULoad();
static unsigned long long FileTimeToInt64();
float GetCPULoad();

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


/*std::vector<ProcessCollection> collectProcesses(){

}

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

void sendCollectionInfo()
{

}

void listener(){
    // Will listen for connection init, query requests

    // A collection event will proc one of the following (collectProcesses, collectPorts, collectResourceUsage)
    // Will spawn a new thread for each collection type. We'll then use sendCollectionInfo to send in a serialised JSON format
}




int main(){
    //std::thread eventListner(listener);


    // Testing resCol
    ResourceUsageCollection test = collectResourceUsage();

    std::cout << "===== RESOURCE USAGE TEST =====\n";
    std::cout << "Agent ID: " << test.agent_id << "\n";
    std::cout << "Source: " << test.source << "\n";
    std::cout << "Timestamp: " << test.timestamp << "\n";
    std::cout << "CPU Usage (%): " << test.cpu_usage_p << "/100\n";
    std::cout << "Total Memory (MB): " << test.total_memory_mb << "\n";
    std::cout << "Used Memory (MB): " << test.used_memory_mb << "\n";
    std::cout << "Free Memory (MB): " << test.free_memory_mb << "\n";
    std::cout << "Memory Usage (%): " << test.memory_usage_p << "\n";
    std::cout << "Uptime (s): " << test.uptime_s << "\n";
    std::cout << "================================\n";
    return 0;
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