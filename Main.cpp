#include<iostream>
#include <string>
#include "ConsoleManager.h";
#include "Commands.h"
#include "FCFSScheduler.h"
#include "Console.h"
#include "Config.h"
#include "RRScheduler.h"
#include <random>
#include "PagingManager.h"
#include "FirstFitManager.h"
#include "MemoryManager.h"

int main() {
    displayHeader();
    std::string command;
    int totalM = 0;
    int usedM = 0;
    int freeM = 0;
    ConsoleManager console_manager;
    bool running = true;

    // Initialize min and max process instructions

    // Initialize scheduler
    FCFS_Scheduler fcfs_scheduler(0);
    RR_Scheduler rr_scheduler(0, 0);


    // Initialize scheduling test
    std::thread scheduler_thread;
    int process_count = 0;
    bool scheduler_testing = false;
    std::string screen_process_name = "";
    bool initialized = false;
    while (running) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        // Input tokens
        std::stringstream ss(command);
        std::vector<std::string> tokens;
        std::string token;
        while (ss >> token) {
            tokens.push_back(token);
        }

        if (command != "initialize" && command != "exit" && initialized == false) {
            if (Config::GetConfigParameters().num_cpu == NULL ||
                Config::GetConfigParameters().scheduler == "" ||
                Config::GetConfigParameters().quantum_cycles == NULL ||
                Config::GetConfigParameters().batch_process_freq == NULL ||
                Config::GetConfigParameters().min_ins == NULL ||
                Config::GetConfigParameters().max_ins == NULL ||
                Config::GetConfigParameters().delay_per_exec == NULL ||
                Config::GetConfigParameters().max_overall_mem == NULL ||
                Config::GetConfigParameters().min_mem_per_proc == NULL ||
                Config::GetConfigParameters().max_mem_per_proc == NULL ||
                Config::GetConfigParameters().min_page_per_proc == NULL ||
                Config::GetConfigParameters().max_page_per_proc == NULL) {
                std::cout << "Initialize the program with command \"initialize\"" << std::endl;
            }
        }

        std::vector<std::shared_ptr<Console>> consoles = console_manager.getConsoles();

        // Current console is not the main menu and the exit command is entered.
        if (command == "exit" && (console_manager.getCurrentConsoleName() != "MAIN_MENU")) {
            system("cls");
            // Find the console with the name "MAIN_MENU"
            for (const auto& consolePtr : consoles) {
                if (consolePtr->getName() == "MAIN_MENU") {
                    console_manager.setCurrentConsole(consolePtr);
                    break;
                }
            }
            // If "MAIN_MENU" console is found, draw its main content
            if (console_manager.getCurrentConsole() && console_manager.getCurrentConsoleName() == "MAIN_MENU") {
                displayHeader();
            }
        }

        // Current console is the main menu and the exit command is entered.
        else if (command == "exit" && console_manager.getCurrentConsoleName() == "MAIN_MENU") {
            return false;
        }
        else if (command == "vmstat") {
            if (Config::GetConfigParameters().scheduler == "fcfs") {
            }
            if (Config::GetConfigParameters().scheduler == "rr") {
                float freeMem = 0, usedMemory = 0;
                float totalMem;
                int memUtil;

                // Checks if memory allocator is first-fit
                if (dynamic_cast<PagingManager*>(rr_scheduler.memoryManager) == nullptr) {
                    FirstFitManager* ffm = dynamic_cast<FirstFitManager*>(rr_scheduler.memoryManager);

                    for (int i = 0; i < ffm->getMemoryBlockList().size(); i++) {
                        if (ffm->getMemoryBlockList()[i] == -1) {
                            freeMem++;
                        }
                        else {
                            usedMemory++;
                        }
                    }

                    totalMem = Config::GetConfigParameters().max_overall_mem;
                }
                else {
                    PagingManager* pm = dynamic_cast<PagingManager*>(rr_scheduler.memoryManager);
                    freeMem = pm->currentFrames * pm->pageSize;
                    totalMem = pm->maxFrames * pm->pageSize;
                    usedMemory = totalMem - freeMem;
                }

                memUtil = (usedMemory / totalMem) * 100;
              

                std::cout << "Total Memory: " << totalMem << "\n";
                std::cout << "Used Memory: " << usedMemory << "\n";
                std::cout << "Free Memory: " << totalMem - usedMemory << "\n";



            }

        }
        else if (command == "process-smi") {
            float cpuUtilization = 0.0f;

            if (Config::GetConfigParameters().scheduler == "fcfs") {
                cpuUtilization = fcfs_scheduler.GetCpuUtilizations();
            }

            if (Config::GetConfigParameters().scheduler == "rr") {
                cpuUtilization = rr_scheduler.GetCpuUtilization();
            }
            float freeMem = 0, usedMemory = 0;
            float totalMem;
            int memUtil;

            // Checks if memory allocator is first-fit
            if (dynamic_cast<PagingManager*>(rr_scheduler.memoryManager) == nullptr) {
                FirstFitManager* ffm = dynamic_cast<FirstFitManager*>(rr_scheduler.memoryManager);

                for (int i = 0; i < ffm->getMemoryBlockList().size(); i++) {
                    if (ffm->getMemoryBlockList()[i] == -1) {
                        freeMem++;
                    }
                    else {
                        usedMemory++;
                    }
                }

                totalMem = Config::GetConfigParameters().max_overall_mem;
            }
            else {
                PagingManager* pm = dynamic_cast<PagingManager*>(rr_scheduler.memoryManager);
                freeMem = pm->currentFrames * pm->pageSize;
                totalMem = pm->maxFrames * pm->pageSize;
                usedMemory = totalMem - freeMem;
            }

            memUtil = (usedMemory / totalMem) * 100;

            std::cout << std::endl;
            std::cout << "-------------------------------------------" << std::endl;
            std::cout << "| PROCESS-SMI V01.00 Driver Version 01.00 |" << std::endl;
            std::cout << "-------------------------------------------" << std::endl;
            std::cout << "CPU-Util: " << cpuUtilization << "%" << std::endl;
            std::cout << "Memory Usage: " << usedMemory << "KB / " << Config::GetConfigParameters().max_overall_mem << "KB" << std::endl;
            std::cout << "Memory Util: " << memUtil << "%" << std::endl;
            std::cout << std::endl;
            std::cout << "===================================" << std::endl;
            std::cout << "Running processes and memory usage:" << std::endl;

            if (Config::GetConfigParameters().scheduler == "fcfs") {
                for (const auto& process : fcfs_scheduler.running_processes) {
                    int proc_mem = 0;
                    if (dynamic_cast<PagingManager*>(rr_scheduler.memoryManager) == nullptr) {
                        proc_mem = process->endAddress - process->startAddress;
                    }
                    else {
                        PagingManager* pm = dynamic_cast<PagingManager*>(rr_scheduler.memoryManager);
                        proc_mem = pm->pageSize * pm->expectedFramesPerProcess;
                    }

                    std::cout << process->name << " " << proc_mem << "KB" << std::endl;
                }
            }

            if (Config::GetConfigParameters().scheduler == "rr") {
                for (const auto& process : rr_scheduler.running_processes) {
                    int proc_mem = 0;
                    if (dynamic_cast<PagingManager*>(rr_scheduler.memoryManager) == nullptr) {
                        proc_mem = process->endAddress - process->startAddress;
                    }
                    else {
                        PagingManager* pm = dynamic_cast<PagingManager*>(rr_scheduler.memoryManager);
                        proc_mem = pm->pageSize * pm->expectedFramesPerProcess;
                    }

                    std::cout << process->name << " " << proc_mem << "KB" << std::endl;
                }
            }
        }

        // Current console is the main menu and the screen -s (create process) is entered.
        else if (tokens[0] == "screen" && tokens[1] == "-s") {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(Config::GetConfigParameters().min_ins, Config::GetConfigParameters().max_ins);

            if (Config::GetConfigParameters().scheduler == "fcfs") {
                if (fcfs_scheduler.isValidProcessName(tokens[2]) == false) {
                    std::cout << "Process with name \"" + tokens[2] + "\" already exists" << std::endl;
                }
                else {
                    int commands_per_process = dist(gen);
                    Process* new_process = new Process(tokens[2], commands_per_process);
                    fcfs_scheduler.add_process(new_process);

                    std::shared_ptr<Console> new_console(new Console(new_process->name, new_process->executed_commands, new_process->total_commands, new_process->process_id));
                    screen_process_name = tokens[2];
                    console_manager.setCurrentConsole(new_console);
                    fcfs_scheduler.print_process_details(tokens[2], 0);

                }
            }

            if (Config::GetConfigParameters().scheduler == "rr") {
                if (rr_scheduler.isValidProcessName(tokens[2]) == false) {
                    std::cout << "Process with name \"" + tokens[2] + "\" already exists" << std::endl;
                }
                else {
                    int commands_per_process = dist(gen);
                    Process* new_process = new Process(tokens[2], commands_per_process);
                    rr_scheduler.add_process(new_process);

                    std::shared_ptr<Console> new_console(new Console(new_process->name, new_process->executed_commands, new_process->total_commands, new_process->process_id));
                    screen_process_name = tokens[2];
                    console_manager.setCurrentConsole(new_console);
                    rr_scheduler.print_process_details(tokens[2], 0);
                }
            }
        }

        else if (command == "show") {
            console_manager.drawAllConsoles();
        }

        // The current console is not the main menu and the screen -s command is entered.
        else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-s" && tokens[2] != "" && console_manager.getCurrentConsoleName() != "MAIN_MENU") {

            std::cout << "Unkown Command" << std::endl;
        }

        // The current ocnsole is not the main menu and the screen -r command is entered.
        else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-r" && tokens[2] != "" && console_manager.getCurrentConsoleName() != "MAIN_MENU") {

            std::cout << "Unkown Command" << std::endl;
        }
        else if (command == "clear") {
            clearScreen();
        }

        else if (console_manager.getCurrentConsoleName() != "MAIN_MENU" && command == "process-smi") {
            if (Config::GetConfigParameters().scheduler == "fcfs") {
                fcfs_scheduler.print_process_details(screen_process_name, 1);

            }

            if (Config::GetConfigParameters().scheduler == "rr") {
                rr_scheduler.print_process_details(screen_process_name, 1);
            }


        }

        else if (command == "initialize") {
            initialized = true;
            Config::Initialize();
            std::cout << "Config initialized with \"" << "config.txt\" parameters" << std::endl;

            if (Config::GetConfigParameters().scheduler == "fcfs") {
                fcfs_scheduler.SetCpuCore(Config::GetConfigParameters().num_cpu);
                fcfs_scheduler.start();
            }

            if (Config::GetConfigParameters().scheduler == "rr") {
                rr_scheduler.SetCpuCore(Config::GetConfigParameters().num_cpu);
                rr_scheduler.SetQuantum(Config::GetConfigParameters().quantum_cycles);
                rr_scheduler.start();
            }
        }

        // "scheduler-stop"
        else if (command == "scheduler-stop") {
            if (scheduler_testing) {
                scheduler_testing = false;
                if (scheduler_thread.joinable()) {
                    scheduler_thread.join();
                }

                std::cout << "Scheduler test execution stopped.\n";
            }
            else {
                std::cout << "Scheduler test is not currently running.\n";
            }
        }

        // "scheduler-test"
        else if (command == "scheduler-test") {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(Config::GetConfigParameters().min_ins, Config::GetConfigParameters().max_ins);

            if (!scheduler_testing) {
                scheduler_testing = true;
                scheduler_thread = std::thread([&]() {
                    while (scheduler_testing) {
                        int commands_per_process = dist(gen);

                        if (Config::GetConfigParameters().scheduler == "fcfs") {
                            fcfs_scheduler.add_process(new Process("process" + std::to_string(++process_count), commands_per_process));
                        }

                        if (Config::GetConfigParameters().scheduler == "rr") {
                            rr_scheduler.add_process(new Process("process" + std::to_string(++process_count), commands_per_process));
                        }




                        std::this_thread::sleep_for(std::chrono::milliseconds((int)(Config::GetConfigParameters().batch_process_freq * 1000)));

                    }
                    });
                scheduler_thread.detach();

                std::cout << "Scheduler test execution started.\n";
            }
            else {
                std::cout << "Scheduler test is already running.\n";
            }
        }

        // "screen -ls"
        else if (tokens[0] == "screen" && tokens[1] == "-ls") {
            if (Config::GetConfigParameters().scheduler == "fcfs") {
                fcfs_scheduler.screen_ls();
            }

            if (Config::GetConfigParameters().scheduler == "rr") {
                rr_scheduler.screen_ls();
            }


        }

        // "screen -r"
        else if (tokens.size() == 3 && tokens[0] == "screen" && tokens[1] == "-r") {
            std::cout << "screen -r " << tokens[2];
            if (console_manager.getCurrentConsoleName() == "MAIN_MENU") {
                if (Config::GetConfigParameters().scheduler == "fcfs") {
                    fcfs_scheduler.print_process_details(tokens[2], 0);
                }

                if (Config::GetConfigParameters().scheduler == "rr") {
                    rr_scheduler.print_process_details(tokens[2], 0);
                }

            }
            else {
                if (Config::GetConfigParameters().scheduler == "fcfs") {
                    fcfs_scheduler.print_process_details(tokens[2], 1);
                }

                if (Config::GetConfigParameters().scheduler == "rr") {
                    rr_scheduler.print_process_details(tokens[2], 1);
                }


            }

            screen_process_name = tokens[2];
            std::shared_ptr<Console> new_console(new Console("VIEW_SCREEN", 0, 0, 0));
            console_manager.setCurrentConsole(new_console);
        }

        else if (command == "report-util") {
            if (Config::GetConfigParameters().scheduler == "fcfs") {
                fcfs_scheduler.ReportUtil();
            }

            if (Config::GetConfigParameters().scheduler == "rr") {
                rr_scheduler.ReportUtil();
            }

        }

        else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    return 0;
}
