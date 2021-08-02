#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdint>
#include <optional>

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete(){
        return graphicsFamily.has_value();
    }
};

#endif // STRUCTS_H
