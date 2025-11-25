import os

class Config:
    SQLALCHEMY_DATABASE_URI = 'postgresql://yaakov:Ydavidovici35@localhost/chess_db'
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    SECRET_KEY = os.urandom(24)
