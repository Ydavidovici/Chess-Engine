from flask import Flask
from flask_sqlalchemy import SQLAlchemy
import os
from dotenv import load_dotenv
import pg8000

# Load environment variables from .env file
load_dotenv()

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URI').replace('postgresql', 'postgresql+pg8000')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

def init_db():
    try:
        connection_uri = os.getenv('DATABASE_URI').replace('postgresql', 'postgresql+pg8000')
        print(f"Connecting to database using URI: {connection_uri}")

        result = db.engine.execute("SELECT current_database()")
        current_db = result.fetchone()[0]
        print(f"Connected to database: {current_db}")

        print("Dropping existing tables if they exist...")
        with db.engine.begin() as conn:
            conn.execute('DROP TABLE IF EXISTS game_moves')
            conn.execute('DROP TABLE IF EXISTS games')
            conn.execute('DROP TABLE IF EXISTS player_stats')
        print("Tables dropped successfully.")

        print("Applying schema from schema.sql...")
        schema_path = os.path.join(os.path.dirname(__file__), 'db/schema.sql')
        if not os.path.exists(schema_path):
            print(f"Schema file does not exist at path: {schema_path}")
            return

        with open(schema_path, 'r') as f:
            schema_sql = f.read()
            print(f"Schema SQL: {schema_sql}")  # Debug print schema SQL
            with db.engine.begin() as conn:
                conn.execute(schema_sql)
        print("Schema applied successfully.")

        # Verify table creation
        print("Verifying table creation...")
        result = db.engine.execute("SELECT table_name FROM information_schema.tables WHERE table_schema='public'")
        tables = result.fetchall()
        print(f"Tables in the database: {tables}")

    except Exception as e:
        print(f"An error occurred while initializing the database: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    with app.app_context():
        print("Creating all tables defined in the models...")
        db.create_all()  # Ensure the tables are created with the correct schema
        db.session.commit()  # Commit changes
        print("All tables created successfully.")
        init_db()
        print("Database initialization complete.")
