#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdint>
#include <vector>

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

    double cpu_usage_p;
    
    uint64_t total_memory_mb;
    uint64_t used_memory_mb;
    uint64_t free_memory_mb;
    double memory_usage_p;

    uint64_t uptime_s;
};


std::vector<ProcessCollection> collectProcesses(){

}

std::vector<PortCollection> collectPorts(){

}

ResourceUsageCollection collectResourceUsage(){

}

void sendCollectionInfo()
{

}

void listener(){
    // Will listen for connection init, query requests

    // A collection event will proc one of the following (collectProcesses, collectPorts, collectResourceUsage)
    //Will spawn a new thread for each collection type. We'll then use sendCollectionInfo to send in a serialised JSON format
}

int main(){
    //std::thread eventListner(listener);
    return 0;
}