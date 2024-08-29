#include "player.h"

Player::Player(Player::Color color) : color(color) {
    // Constructor implementation...
}

// Correct usage of scope resolution for nested enum type
Player::Color Player::getColor() const {
    return color;
}
