#include "Scheduler.hpp"
#include "utils.hpp"

int main() {
    std::unique_ptr<Scheduler> s = std::make_unique<Scheduler>();
        
    std::cout << "Scheduler is running. Press ENTER to stop..." << std::endl;

    std::cin.get();

    s->shut_down();

    s.reset();

    std::cout << "Scheduler stopped." << std::endl;
    return (int) core::ReturnCode::SUCCESS;
}
