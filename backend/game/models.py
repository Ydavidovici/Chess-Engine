# backend/game/models.py

from django.db import models

class Game(models.Model):
    id = models.AutoField(primary_key=True)
    position = models.JSONField()
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

class Move(models.Model):
    id = models.AutoField(primary_key=True)
    game = models.ForeignKey(Game, related_name='moves', on_delete=models.CASCADE)
    move = models.CharField(max_length=10)
    timestamp = models.DateTimeField(auto_now_add=True)
