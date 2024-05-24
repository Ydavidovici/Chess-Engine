// backend/engine/include/player.h

#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
    enum Color { WHITE, BLACK };

    Player(Color color);
    Color getColor() const;

private:
    Color color;
};

#endif // PLAYER_H
