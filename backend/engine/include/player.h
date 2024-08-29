#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
    enum class Color {
        WHITE,
        BLACK
    };

    Player(Color color);

    Color getColor() const;  // Correctly use the nested Color type

private:
    Color color;
    // Other member variables...
};

#endif // PLAYER_H
