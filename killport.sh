#!/bin/bash

kill_port_process() {
    local port=$1
    echo "Find and kill processes occupying port $port"
    local pids=$(sudo lsof -t -i:$port)
    if [ -z "$pids" ]; then
        echo "No process occupying port $port was found."
    else
        for pid in $pids; do
            echo "kill process ID $pid..."
            sudo kill -9 $pid
        done
    fi
}

kill_port_process 443

kill_port_process 80

echo "finish"