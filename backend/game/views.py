# backend/game/views.py

from django.http import JsonResponse
from .models import Game, Move
import engine  # Import the compiled C++ engine

def create_game(request):
    initial_position = {}  # Define the initial position
    game = Game.objects.create(position=initial_position)
    board = engine.Board()
    board.initializeBoard()
    # Store initial position from C++ engine
    return JsonResponse({'game_id': game.id})

def make_move(request, game_id):
    game = Game.objects.get(id=game_id)
    move = request.POST['move']
    # Use C++ engine to validate and apply move
    Move.objects.create(game=game, move=move)
    return JsonResponse({'status': 'move made'})

def game_status(request, game_id):
    game = Game.objects.get(id=game_id)
    return JsonResponse({"game_id": game.id, "position": game.position, "moves": list(game.moves.values())})
