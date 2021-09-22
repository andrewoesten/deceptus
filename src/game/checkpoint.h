#pragma once

#include "SFML/Graphics.hpp"

#include <functional>
#include <vector>


struct TmxObject;


class Checkpoint
{

public:

    using CheckpointCallback = std::function<void(void)>;

    static Checkpoint* getCheckpoint(uint32_t index);
    static uint32_t add(TmxObject*);
    static void update();
    static void resetAll();

    void reached();
    void addCallback(CheckpointCallback);
    sf::Vector2i calcCenter() const;


private:

    Checkpoint() = default;

    uint32_t _index = 0;
    std::string _name;

    sf::IntRect _rect;
    bool _reached = false;

    std::vector<CheckpointCallback> _callbacks;

    static std::vector<Checkpoint> __checkpoints;
};

