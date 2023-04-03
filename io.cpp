#include "deps/nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

int main(int argc, char** argv)
{
    json data = {
        {"test_i", 35},
        {"test_b", 29}
    };
    std::vector<uint8_t> bin = json::to_bjdata(data);
    std::ofstream file("out.bin", std::ios::out | std::ios::binary);
    file << bin.data();
    file.flush();
    file.close();
    
    bin.clear();
    std::ifstream in("out.bin", std::ios::in | std::ios::binary);
    data = json::from_bjdata(in);
    in.close();
    std::cout << "test_i: " << data["test_i"] << ", test_b: " << data["test_b"] << '\n';
    return 0;
}