# Use an official Node.js runtime as a parent image
FROM node:18-alpine

# Set the working directory
WORKDIR /app/frontend

# Copy the frontend code into the container
COPY frontend/package*.json ./
COPY frontend ./

# Install the dependencies
RUN npm install

# Expose the port the app runs on
EXPOSE 3000

# Start the development server
CMD ["npm", "run", "dev"]
