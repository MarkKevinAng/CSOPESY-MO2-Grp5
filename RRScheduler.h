#pragma once

#include "MemoryManager.h"
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Process.h"
#include <list>

class RR_Scheduler {
private:
    bool isStealing = false;
    int num_cores;
    int time_quantum;
    bool running;
    std::vector<std::thread> cpu_threads;
    std::queue<Process*> process_queue;
    std::mutex mtx;
    std::mutex stealMtx;
    std::condition_variable cv;
    std::condition_variable stealCv;
    std::chrono::steady_clock::time_point start_time;
    std::list<Process*> finished_processes;
    void cpu_worker(int core_id);

public:
    std::vector<int> idle_ticks_per_core;    // Track idle ticks for each core
    std::vector<int> active_ticks_per_core;  // Track active ticks for each core
    std::vector<int> total_ticks_per_core;   // Track total ticks for each core
    void display_cpu_stats();
    int idleT = 0;
    int activeT = 0;
    int totalT = 0;
    
    MemoryManager* memoryManager = nullptr;
    RR_Scheduler(int cores, int quantum);
    ~RR_Scheduler();
    void add_process(Process* proc);
    void start();
    void stop();
    void print_running_processes();
    void print_finished_processes();
    void print_process_details(const std::string& process_name, int screen);
    void screen_ls();
    void print_process_queue_names();
    void SetCpuCore(int cpu_core);
    void SetQuantum(int quantum);
    bool isValidProcessName(const std::string& process_name);
    void ReportUtil();
    void print_CPU_UTIL();

    std::list<Process*> running_processes;
    int GetCpuUtilization();
};