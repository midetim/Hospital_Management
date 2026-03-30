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
sleep 2
echo -e "\033[32mDocker Compose setup complete.\033[0m"

SERVICES=(
    roommanagement
    patientmanagement
    resourcemanagement
    staffmanagement
    scheduler
)

open_terminal() {
    local CMD="$1"

    # macOS
    if [[ "$OSTYPE" == "darwin"* ]]; then
        osascript <<EOF
tell application "Terminal"
    do script "$CMD"
    activate
end tell
EOF

    # Linux
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v gnome-terminal >/dev/null 2>&1; then
            gnome-terminal -- bash -c "$CMD; exec bash"
        elif command -v konsole >/dev/null 2>&1; then
            konsole -e bash -c "$CMD; exec bash"
        elif command -v xterm >/dev/null 2>&1; then
            xterm -e "$CMD"
        else
            echo "No terminal emulator found. Running in current shell."
            bash -c "$CMD"
        fi

    # Windows (Git Bash)
    elif [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* ]]; then
        cmd.exe /c start bash -lc "$CMD"

    else
        bash -c "$CMD"
    fi
}

# Step 2 -- Launch mode
case "$MODE" in
    cli)
        for SERVICE in "${SERVICES[@]}"; do
            CONTAINER=$(docker compose ps -q "$SERVICE")
            
            if [ -z "$CONTAINER" ]; then
                echo -e "\033[31mContainer for $SERVICE not found!\033[0m"
                continue
            fi
            
            echo -e "\033[36mOpening terminal for $SERVICE...\033[0m"
            
            CMD="docker attach $CONTAINER"
            
            open_terminal "$CMD"
        done
            
        docker attach frontend_cli
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
