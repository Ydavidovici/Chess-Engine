# Use an official Python runtime as a parent image
FROM python:3.9-slim

# Install the necessary packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    build-essential \
    cmake \
    libpython3-dev && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the backend code into the container
COPY backend /app/backend
COPY backend/engine/include /app/backend/engine/include
COPY backend/engine/src /app/backend/engine/src
COPY backend/engine/CMakeLists.txt /app/backend/engine/CMakeLists.txt
COPY backend/requirements.txt /app/requirements.txt
COPY CMakeLists.txt /app/CMakeLists.txt

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Build the C++ engine
RUN cd /app && mkdir build && cd build && cmake -DCMAKE_PREFIX_PATH=$(python3 -m pybind11 --cmakedir) .. && make

# Copy the compiled shared object files
RUN cp /app/build/backend/engine/engine.*.so /app/backend/engine/ || cp /app/build/engine.*.so /app/backend/engine/

# Set environment variables for the Flask app
ENV FLASK_APP=backend/app.py
ENV FLASK_RUN_HOST=0.0.0.0

# Expose the port the app runs on
EXPOSE 5000

# Run the Flask app
CMD ["flask", "run"]
