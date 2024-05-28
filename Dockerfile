# Use an official Python runtime as a parent image
FROM python:3.9-slim as backend

# Install the necessary certificates
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the backend code into the container
COPY backend/requirements.txt backend/
COPY backend /app/backend

# Install the dependencies with trusted hosts to bypass SSL verification
RUN pip install --no-cache-dir --trusted-host pypi.org --trusted-host files.pythonhosted.org -r backend/requirements.txt

# Set environment variables for the Flask app
ENV FLASK_APP=backend/app.py
ENV FLASK_RUN_HOST=0.0.0.0

# Run the Flask app
CMD ["flask", "run"]

# Use an official Node.js runtime as a parent image
FROM node:18-alpine as frontend

# Set the working directory
WORKDIR /app/frontend

# Copy the frontend code into the container
COPY frontend/package*.json ./
COPY frontend ./

# Install the dependencies
RUN npm install

# Start the development server
CMD ["npm", "run", "dev"]
