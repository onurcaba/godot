#!/usr/bin/env bash
# ==============================================================================
# CBT Content Studio - Standalone Project Launcher
# ==============================================================================

set -e

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EDITOR_BIN="$REPO_DIR/bin/godot.macos.editor.arm64"
DEFAULT_POC="/Users/onur/Documents/GitHub/cbt-authoring-poc"

if [ ! -f "$EDITOR_BIN" ]; then
    echo "Error: CBT Content Studio binary not found at:"
    echo "  $EDITOR_BIN"
    echo "Please build the editor first using SCons."
    exit 1
fi

scaffold_cbt_project() {
    local target_dir="$1"
    local proj_name="$2"

    mkdir -p "$target_dir"

    # 1. Scaffolding project.json
    local pj="$target_dir/project.json"
    if [ ! -f "$pj" ]; then
        cat <<EOF > "$pj"
{
  "projectId": "project.$(echo "$proj_name" | tr '[:upper:]' '[:lower:]' | tr ' ' '_')",
  "name": "$proj_name",
  "description": "CBT Training Simulation Project",
  "schemaVersion": 4,
  "contentVersion": "1.0.0",
  "author": "CBT Content Studio",
  "targetRuntimes": ["VR", "Desktop"]
}
EOF
        echo "  Created $pj"
    fi

    # 2. Scaffolding project.godot
    local pg="$target_dir/project.godot"
    if [ ! -f "$pg" ]; then
        cat <<EOF > "$pg"
; Engine configuration file for CBT Content Studio
; Packaging and contracts follow CBT Content Studio v4 specification

config_version=5

[application]

config/name="$proj_name"
config/features=PackedStringArray("4.6", "Forward Plus")

[rendering]

renderer/rendering_method="forward_plus"
EOF
        echo "  Created $pg"
    fi
}

launch_project() {
    local project_path="$1"
    echo ""
    echo "=========================================================="
    echo " Launching CBT Content Studio"
    echo " Project: $project_path"
    echo "=========================================================="
    echo ""
    exec "$EDITOR_BIN" --path "$project_path" -e
}

# ------------------------------------------------------------------------------
# Command Line Argument Handling
# ------------------------------------------------------------------------------

if [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
    echo "Usage: ./run_launcher.sh [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --path <dir>            Directly launch project in <dir>"
    echo "  --new <dir> <name>      Scaffold new CBT project and launch"
    echo "  --poc                   Launch default CBT Authoring POC"
    echo "  --help                  Show this help message"
    echo ""
    exit 0
fi

if [ "$1" == "--path" ] && [ -n "$2" ]; then
    launch_project "$2"
fi

if [ "$1" == "--poc" ]; then
    launch_project "$DEFAULT_POC"
fi

if [ "$1" == "--new" ] && [ -n "$2" ] && [ -n "$3" ]; then
    scaffold_cbt_project "$2" "$3"
    launch_project "$2"
fi

# ------------------------------------------------------------------------------
# Interactive Launcher Menu
# ------------------------------------------------------------------------------

echo "=========================================================="
echo "          CBT CONTENT STUDIO - PROJECT LAUNCHER           "
echo "=========================================================="
echo ""
echo "Select an option:"
echo "  [1] Open Default POC Project ($DEFAULT_POC)"
echo "  [2] Create New CBT Project (Scaffold project.json + project.godot)"
echo "  [3] Open Existing CBT Project Directory"
echo "  [4] Launch Editor Directly"
echo "  [q] Quit"
echo ""
read -p "Select [1-4, q]: " choice

case "$choice" in
    1)
        launch_project "$DEFAULT_POC"
        ;;
    2)
        echo ""
        read -p "Enter new project name: " new_name
        read -p "Enter parent directory path: " parent_dir
        if [ -z "$new_name" ] || [ -z "$parent_dir" ]; then
            echo "Invalid project name or directory."
            exit 1
        fi
        target_path="$parent_dir/$new_name"
        echo "Scaffolding CBT Project at $target_path..."
        scaffold_cbt_project "$target_path" "$new_name"
        launch_project "$target_path"
        ;;
    3)
        echo ""
        read -p "Enter project directory path: " custom_path
        if [ ! -d "$custom_path" ]; then
            echo "Directory does not exist: $custom_path"
            exit 1
        fi
        launch_project "$custom_path"
        ;;
    4)
        exec "$EDITOR_BIN" -e
        ;;
    q|Q)
        echo "Exiting."
        exit 0
        ;;
    *)
        echo "Defaulting to CBT POC Project..."
        launch_project "$DEFAULT_POC"
        ;;
esac
