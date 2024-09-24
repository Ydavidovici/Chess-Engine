class MockChessEngine:
    def __init__(self):
        self.board_state = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    def make_move(self, move):
        # Simplified move application for testing
        self.board_state = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 1 1"  # Example state after 1.e4

    def get_board_state(self):
        return self.board_state

mock_chess_engine = MockChessEngine()
