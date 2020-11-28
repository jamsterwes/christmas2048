#pragma once
#include <cinttypes>
#include <string>

enum class MoveID : uint8_t
{
    LEFT = 0,
    RIGHT,
    UP,
    DOWN
};

inline std::ostream& operator<<(std::ostream& stream, const MoveID& id)
{
    switch (id)
    {
    case MoveID::LEFT:
        return stream << std::string("LEFT");
    case MoveID::UP:
        return stream << std::string("UP");
    case MoveID::DOWN:
        return stream << std::string("DOWN");
    case MoveID::RIGHT:
        return stream << std::string("RIGHT");
    default:
        return stream << std::string("UNKNOWN");
    }
}