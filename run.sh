#!/usr/bin/env bash

# Step 1 -- Check input
if [ "$#" -ne 1 ]; then
    echo -e "\033[31mUsage: $0 [cli|gui]\033[0m"
    exit 1
fi

MODE=$1

# Step 1 -- Docker Compose
echo -e "\033[36mBuilding and starting Docker Compose...\033[0m"
docker compose down
docker compose up --build -d
echo -e "\033[32mDocker Compose setup complete.\033[0m"

# Step 2 -- Launch mode
case "$MODE" in
    cli)
        # Replace 'your_container_name' with the actual container name
        CONTAINER=$(docker compose ps -q frontend_cli)
        if [ -z "$CONTAINER" ]; then
            echo -e "\033[31mError: container for service 'frontend_cli' not found!\033[0m"
            exit 1
        else
            echo -e "\033[32mAttaching to container $CONTAINER...\033[0m"
        fi
        docker attach "$CONTAINER"
        ;;
    gui)
        URL="http://localhost:8920"
        echo -e "\033[32mOpening GUI at \033[35m$URL...\033[0m"
        # macOS
        if [[ "$OSTYPE" == "darwin"* ]]; then
            open "$URL"
        # Linux
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            xdg-open "$URL"
        else
            echo -e "\033[33mPlease open \033[35m$URL\033[33m manually in your browser.\033[0m"
        fi
        ;;
    *)
        echo -e "\033[31mInvalid mode: $MODE. Use 'cli' or 'gui'.\033[0m"
        exit 1
        ;;
esac