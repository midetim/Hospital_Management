#include "Scheduler.hpp"

#include <atomic>
#include <csignal>
#include <thread>
#include <chrono>

std::atomic<bool> running = true;

void signalHandler(int signum) {
    running = false;
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::unique_ptr<Scheduler> s = std::make_unique<Scheduler>();

    while(running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Signal received. Uploading DB before shutdown...\n";

    s->shut_down();

    s.reset();

    std::cout << "Scheduler stopped." << std::endl;
    return (int) core::ReturnCode::SUCCESS;
}

