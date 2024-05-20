# backend/lichess_api.py

import requests

class LichessAPI:
    def __init__(self, token):
        self.base_url = "https://lichess.org/api"
        self.headers = {
            "Authorization": f"Bearer {token}",
            "Content-Type": "application/json"
        }

    def get_account(self):
        url = f"{self.base_url}/account"
        response = requests.get(url, headers=self.headers)
        return response.json()

    def get_current_games(self):
        url = f"{self.base_url}/account/playing"
        response = requests.get(url, headers=self.headers)
        return response.json()

    def make_move(self, game_id, move):
        url = f"{self.base_url}/bot/game/{game_id}/move/{move}"
        response = requests.post(url, headers=self.headers)
        return response.json()
