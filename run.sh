#!/bin/bash

# Step 1 -- Check input
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 [cli|gui]"
    exit 1
fi

MODE=$1

# Step 1 -- Docker Compose
echo "Building and starting Docker Compose"
docker compose down
docker compose up --build -d
echo "Docker Compose setup complete"

# Step 2 -- Launch mode
case "$MODE" in
    cli)
        # Replace 'your_container_name' with the actual container name
        CONTAINER=$(docker compose ps -q frontend_cli)
        if [ -z "$CONTAINER" ]; then
            echo "Error: container for service 'frontend_cli' not found!"
            exit 1
        else
            echo "Attaching to container $CONTAINER..."
        fi
        docker attach "$CONTAINER"
        ;;
    gui)
        URL="http://localhost:8920"
        echo "Opening GUI at $URL..."
        # macOS
        if [[ "$OSTYPE" == "darwin"* ]]; then
            open "$URL"
        # Linux
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            xdg-open "$URL"
        else
            echo "Please open $URL manually in your browser at localhost:8920."
        fi
        ;;
    *)
        echo "Invalid mode: $MODE. Use 'cli' or 'gui'."
        exit 1
        ;;
esac