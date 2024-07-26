import pg8000
from pg8000 import OperationalError

def create_connection():
    try:
        connection = pg8000.connect(
            user="yaakov",
            password="Ydavidovici35",
            host="localhost",
            port=5432,
            database="chess_db"
        )
        print("Connection to PostgreSQL DB successful")
    except OperationalError as e:
        print(f"The error '{e}' occurred")
        import traceback
        traceback.print_exc()

create_connection()
