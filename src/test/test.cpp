//
// Created by MSI on 2024/6/6.
//
#include <iostream>
#include <unordered_map>
#include <string>
int main()
{
    std::unordered_map<std::string, int> u = {
            {"green", 0},
            {"red", 1}
    };
    auto print_key_value = [](const auto&key, const auto&value) {
        std::cout << "Key:" << key << ",Value:" << value << std::endl;
    };
    for (const auto& [key, val]: u)
    {
        print_key_value(key, val);
    }
}
