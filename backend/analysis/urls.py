# backend/analysis/urls.py

from django.urls import path
from .views import analyze_game

urlpatterns = [
    path('analyze/<int:game_id>/', analyze_game, name='analyze_game'),
]
