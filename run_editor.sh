#!/usr/bin/env bash
# Godot CBT Editor Launch Script

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EDITOR_BIN="$REPO_DIR/bin/godot.macos.editor.arm64"
POC_PROJECT_DIR="/Users/onur/Documents/GitHub/cbt-authoring-poc"

if [ ! -f "$EDITOR_BIN" ]; then
    echo "Hata: Editör ikilisi bulunamadı: $EDITOR_BIN"
    echo "Lütfen önce SCons ile derleyin."
    exit 1
fi

# If no arguments provided, launch the CBT Project Launcher
if [ $# -eq 0 ]; then
    echo "CBT Content Studio Launcher başlatılıyor..."
    "$EDITOR_BIN"
elif [ "$1" == "--poc" ]; then
    echo "CBT Content Studio (POC Projesi) başlatılıyor..."
    echo "Proje: $POC_PROJECT_DIR"
    "$EDITOR_BIN" --path "$POC_PROJECT_DIR" -e
else
    # Forward arguments (e.g. --path <path> -e, --help, etc.)
    "$EDITOR_BIN" "$@"
fi
