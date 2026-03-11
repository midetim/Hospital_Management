#include "FrontEndService.hpp"

int main() {
    FrontEndService f;
    f.read_input();
    return (int) ReturnCode::SUCCESS;
}
