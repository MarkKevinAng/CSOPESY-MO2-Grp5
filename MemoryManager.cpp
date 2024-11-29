#include "MemoryManager.h"
#include "FirstFitManager.h"
#include "Config.h"


int MemoryManager::getTotalMemory() {
    return this->totalMemory;
}

void MemoryManager::store_process(Process* process) {
    std::lock_guard<std::mutex> lock(this->fileMutex);
    process->isBackingStored = true;
    std::string process_store_path = std::to_string(process->process_id) + ".txt";

    std::ofstream process_store(process_store_path);
    if (!process_store.is_open()) {
        std::cout << "Error opening file " << process_store_path << std::endl;
        return;
    }

    process_store << "Process ID: " << process->process_id << std::endl;
    process_store << "Process Name: " << process->name << std::endl;
    process_store << "Command Counter: " << process->executed_commands << std::endl;
    process_store.close();
}

MemoryManager::process_data MemoryManager::load_process(Process* process) {
    std::lock_guard<std::mutex> lock(this->fileMutex);
    process->isBackingStored = false;
    std::string process_store_path = std::to_string(process->process_id) + ".txt";

    std::ifstream process_store(process_store_path);
    if (!process_store.is_open()) {
        std::cout << "Error opening file " << process_store_path << std::endl;
        return {};
    }

    process_data pd;

    std::string line;
    while (std::getline(process_store, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, ':')) {
            std::string value;
            if (std::getline(iss, value)) {
                value.erase(0, value.find_first_not_of(' '));
                if (key == "Process ID") {
                    pd.process_id = std::stoi(value);
                }
                else if (key == "Process Name") {
                    pd.name = value;
                }
                else if (key == "Command Counter") {
                    pd.executed_commands = std::stoi(value);
                }
                else if (key == "Number of Pages") {
                    pd.pages = std::stoi(value);
                }
            }
        }
    }
    process_store.close();

    if (std::remove(process_store_path.c_str()) != 0) {
        std::cout << "Error deleting file " << process_store_path << std::endl;
    }

    process->process_id = pd.process_id;
    process->name = pd.name;
    process->executed_commands = pd.executed_commands;

    return pd;
}