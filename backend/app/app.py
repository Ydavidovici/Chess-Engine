import os
import logging
from datetime import datetime
from dotenv import load_dotenv
from fastapi import FastAPI
from .engine_manager import EngineManager
import requests

load_dotenv()

# # ------------------------------
# # Database Setup
# # ------------------------------
# DATABASE_URL = os.getenv('DATABASE_URI').replace('postgresql', 'postgresql+pg8000')
# engine = create_engine(DATABASE_URL, pool_pre_ping=True)
# SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
# Base = declarative_base()
#
#
# # ------------------------------
# # Models
# # ------------------------------
# class PlayerStats(Base):
#     __tablename__ = 'player_stats'
#     id = Column(Integer, primary_key=True, index=True)
#     name = Column(String, unique=True, nullable=False)
#
#
# class Game(Base):
#     __tablename__ = 'games'
#     game_id = Column(Integer, primary_key=True, index=True)
#     start_time = Column(DateTime, default=datetime.utcnow)
#     end_time = Column(DateTime, nullable=True)
#     result = Column(String, nullable=True)
#     player1_name = Column(String, ForeignKey("player_stats.name"))
#     player2_name = Column(String, ForeignKey("player_stats.name"))
#
#
# class GameMove(Base):
#     __tablename__ = 'game_moves'
#     id = Column(Integer, primary_key=True, index=True)
#     game_id = Column(Integer, ForeignKey("games.game_id"))
#     move_number = Column(Integer, nullable=False)
#     move_notation = Column(String, nullable=False)
#
#
# # ------------------------------
# # Database Utility Functions
# # ------------------------------
# def init_db():
#     Base.metadata.create_all(bind=engine)
#
#
# def get_db():
#     db = SessionLocal()
#     try:
#         yield db
#     finally:
#         db.close()
#
#
# # ------------------------------
# # Lichess API Integration
# # ------------------------------
# class LichessAPI:
#     def __init__(self, token):
#         self.base_url = "https://lichess.org/api"
#         self.headers = {
#             "Authorization": f"Bearer {token}",
#             "Content-Type": "application/json"
#         }
#
#     def get_account(self):
#         response = requests.get(f"{self.base_url}/account", headers=self.headers)
#         return response.json()
#
#     def get_current_games(self):
#         response = requests.get(f"{self.base_url}/account/playing", headers=self.headers)
#         return response.json()
#
#     def make_move(self, game_id, move):
#         response = requests.post(f"{self.base_url}/bot/game/{game_id}/move/{move}", headers=self.headers)
#         return response.json()
#
#
# # ------------------------------
# # Game Manager
# # ------------------------------
# class GameManager:
#     def __init__(self, token: str):
#         self.lichess_api = LichessAPI(token)
#         self.eng = Engine()
#         logging.debug("Chess engine initialized")
#
#     def start_game(self, db: Session, player1_name: str, player2_name: str):
#         # Ensure players exist in database
#         player1 = db.query(PlayerStats).filter(PlayerStats.name == player1_name).first()
#         if not player1:
#             player1 = PlayerStats(name=player1_name)
#             db.add(player1)
#
#         player2 = db.query(PlayerStats).filter(PlayerStats.name == player2_name).first()
#         if not player2:
#             player2 = PlayerStats(name=player2_name)
#             db.add(player2)
#
#         db.commit()
#
#         # Create a new game
#         new_game = Game(player1_name=player1_name, player2_name=player2_name)
#         db.add(new_game)
#         db.commit()
#         logging.debug(f"New game created: {new_game}")
#
#         # Initialize engine and return the starting position
#         self.eng.initialize()
#         return new_game, self.eng.get_board_state()
#
#     def make_move(self, db: Session, game_id: int, move: str):
#         game = db.query(Game).filter(Game.game_id == game_id).first()
#         if not game:
#             raise HTTPException(status_code=404, detail="Game not found")
#
#         # Replay previous moves to reach the current state
#         moves = db.query(GameMove).filter(GameMove.game_id == game_id).order_by(GameMove.move_number).all()
#         for game_move in moves:
#             self.eng.make_move(chess_engine.Move.from_notation(game_move.move_notation), chess_engine.Color.WHITE)
#
#         # Make the new move
#         move_obj = chess_engine.Move.from_notation(move)
#         self.eng.make_move(move_obj, chess_engine.Color.WHITE)
#
#         # Save the move in the database
#         new_move = GameMove(game_id=game_id, move_number=len(moves) + 1, move_notation=move)
#         db.add(new_move)
#         db.commit()
#         logging.debug(f"Move made: {new_move}")
#
#         return self.eng.get_board_state()
#
#     def get_game_status(self, db: Session, game_id: int):
#         game = db.query(Game).filter(Game.game_id == game_id).first()
#         if not game:
#             raise HTTPException(status_code=404, detail="Game not found")
#
#         moves = db.query(GameMove).filter(GameMove.game_id == game_id).order_by(GameMove.move_number).all()
#         for game_move in moves:
#             self.eng.make_move(chess_engine.Move.from_notation(game_move.move_notation), chess_engine.Color.WHITE)
#
#         return {
#             'game_id': game.game_id,
#             'start_time': game.start_time,
#             'end_time': game.end_time,
#             'result': game.result,
#             'player1_name': game.player1_name,
#             'player2_name': game.player2_name,
#             'board': self.eng.get_board_state(),
#             'moves': [{'move_number': m.move_number, 'move_notation': m.move_notation} for m in moves]
#         }
#
#

app = FastAPI()
engine_manager = EngineManager()
# # game_manager = GameManager(os.getenv('LICHESS_TOKEN'))
#
#
# # ------------------------------
# # API Endpoints
# # ------------------------------
# @app.post('/start_game')
# def start_game(player1_name: str, player2_name: str, db: Session = Depends(get_db)):
#     try:
#         new_game, initial_fen = game_manager.start_game(db, player1_name, player2_name)
#         return {"game_id": new_game.game_id, "fen": initial_fen}
#     except Exception as e:
#         logging.exception("Error starting game")
#         raise HTTPException(status_code=500, detail=str(e))
#
#
# @app.post('/make_move')
# def make_move(game_id: int, move: str, db: Session = Depends(get_db)):
#     try:
#         new_fen = game_manager.make_move(db, game_id, move)
#         return {"fen": new_fen}
#     except HTTPException as e:
#         raise e
#     except Exception as e:
#         logging.exception("Error making move")
#         raise HTTPException(status_code=500, detail=str(e))
#
#
# @app.get('/game_status/{game_id}')
# def game_status(game_id: int, db: Session = Depends(get_db)):
#     try:
#         status = game_manager.get_game_status(db, game_id)
#         return status
#     except HTTPException as e:
#         raise e
#     except Exception as e:
#         logging.exception("Error fetching game status")
#         raise HTTPException(status_code=500, detail=str(e))

@app.on_event("startup")
async def startup():
    print("🔥 FastAPI backend started successfully")
    print(f"🔥 Engines available: {engine_manager.list_engines()}")

@app.get("/health")
def health_check():
    return {
        "status": "ok",
        "engines": engine_manager.list_engines(),
    }



if __name__ == "__app__":
    logging.debug("Starting FastAPI application")
    # init_db()
