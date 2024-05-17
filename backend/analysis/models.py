# backend/analysis/models.py

from django.db import models

class Analysis(models.Model):
    game = models.ForeignKey('game.Game', on_delete=models.CASCADE)
    analysis_data = models.JSONField()
    created_at = models.DateTimeField(auto_now_add=True)
