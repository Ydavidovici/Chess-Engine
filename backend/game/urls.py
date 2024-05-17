# backend/game/urls.py

from django.urls import path
from .views import create_game, make_move, game_status

urlpatterns = [
    path('create/', create_game, name='create_game'),
    path('move/<int:game_id>/', make_move, name='make_move'),
    path('status/<int:game_id>/', game_status, name='game_status'),
]
