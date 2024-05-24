// backend/engine/src/player.cpp

#include "player.h"

Player::Player(Color color) : color(color) {}

Player::Color Player::getColor() const {
    return color;
}
