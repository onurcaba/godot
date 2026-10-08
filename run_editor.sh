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

# Eğer bir argüman verilmediyse doğrudan POC projesini editörde açar
if [ $# -eq 0 ]; then
    echo "CBT Content Studio (POC) başlatılıyor..."
    echo "Proje: $POC_PROJECT_DIR"
    "$EDITOR_BIN" --path "$POC_PROJECT_DIR" -e
else
    # Argüman verildiyse (örn: --help, veya başka proje yolu) iletir
    "$EDITOR_BIN" "$@"
fi
