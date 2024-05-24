# Use an official Python runtime as a parent image
FROM python:3.9-slim as backend

# Set the working directory
WORKDIR /app

# Copy the backend code into the container
COPY backend/requirements.txt backend/
COPY backend /app/backend

# Install the dependencies
RUN pip install --no-cache-dir -r backend/requirements.txt

# Set environment variables for the Flask app
ENV FLASK_APP=backend/app.py
ENV FLASK_RUN_HOST=0.0.0.0

# Run the Flask app
CMD ["flask", "run"]

# Use an official Node.js runtime as a parent image
FROM node:14-alpine as frontend

# Set the working directory
WORKDIR /app

# Copy the frontend code into the container
COPY frontend/package*.json frontend/
COPY frontend /app/frontend

# Install the dependencies
RUN npm install

# Build the frontend
RUN npm run build

# Use the official nginx image to serve the frontend
FROM nginx:alpine as production

# Copy the built frontend from the previous stage
COPY --from=frontend /app/frontend/out /usr/share/nginx/html

# Copy the backend stage to production
COPY --from=backend /app /app

# Expose ports
EXPOSE 80 5000

# Start the Flask app and nginx
CMD ["sh", "-c", "flask run & nginx -g 'daemon off;'"]
