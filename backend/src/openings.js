export const OPENINGS = {
    // Categories
    "random_tactical": { name: "Random Tactical", type: "category", style: "tactical" },
    "random_positional": { name: "Random Positional", type: "category", style: "positional" },

    // Specific Openings
    "e4": {
        name: "King's Pawn Game",
        fen: "startpos",
        moves: ["e2e4"],
        style: "tactical"
    },
    "d4": {
        name: "Queen's Pawn Game",
        fen: "startpos",
        moves: ["d2d4"],
        style: "positional"
    },
    "c4": {
        name: "English Opening",
        fen: "startpos",
        moves: ["c2c4"],
        style: "positional"
    },
    "nf3": {
        name: "Zukertort Opening",
        fen: "startpos",
        moves: ["g1f3"],
        style: "positional"
    },
    "sicilian": {
        name: "Sicilian Defense",
        fen: "startpos",
        moves: ["e2e4", "c7c5"],
        style: "tactical"
    },
    "french": {
        name: "French Defense",
        fen: "startpos",
        moves: ["e2e4", "e7e6"],
        style: "positional"
    },
    "caro_kann": {
        name: "Caro-Kann Defense",
        fen: "startpos",
        moves: ["e2e4", "c7c6"],
        style: "positional"
    },
    "ruy_lopez": {
        name: "Ruy Lopez",
        fen: "startpos",
        moves: ["e2e4", "e7e5", "g1f3", "b8c6", "f1b5"],
        style: "tactical"
    },
    "italian": {
        name: "Italian Game",
        fen: "startpos",
        moves: ["e2e4", "e7e5", "g1f3", "b8c6", "f1c4"],
        style: "tactical"
    },
    "queens_gambit": {
        name: "Queen's Gambit",
        fen: "startpos",
        moves: ["d2d4", "d7d5", "c2c4"],
        style: "positional"
    },
    "kings_indian": {
        name: "King's Indian Defense",
        fen: "startpos",
        moves: ["d2d4", "g8f6", "c2c4", "g7g6"],
        style: "tactical"
    },
    "nimzo_indian": {
        name: "Nimzo-Indian Defense",
        fen: "startpos",
        moves: ["d2d4", "g8f6", "c2c4", "e7e6", "b1c3", "f8b4"],
        style: "positional"
    }
};
