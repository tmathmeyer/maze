if [ ! -d ~/.wp ]; then
    mkdir ~/.wp
fi
TIME=$(date +"%H%M%S")
maze -o ~/.wp/$TIME.bmp
feh --bg-tile ~/.wp/$TIME.bmp
