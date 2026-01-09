#!/bin/bash
# USD Vulkan Engine - Cleanup Script
# Removes build artifacts, temporary files, and intermediate files

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get project root directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${GREEN}=== USD Vulkan Engine Cleanup Script ===${NC}"
echo ""

# Function to remove a file/directory if it exists
remove_if_exists() {
    if [ -e "$1" ]; then
        echo -e "${YELLOW}Removing: $1${NC}"
        rm -rf "$1"
    fi
}

# Function to find and remove files by pattern
find_and_remove() {
    local dir="$1"
    local pattern="$2"
    local desc="$3"

    if [ -d "$dir" ]; then
        local count=$(find "$dir" -name "$pattern" 2>/dev/null | wc -l)
        if [ "$count" -gt 0 ]; then
            echo -e "${YELLOW}Removing $count $desc from $dir${NC}"
            find "$dir" -name "$pattern" -delete 2>/dev/null || true
        fi
    fi
}

# Parse command line arguments
CLEAN_LEVEL="tidy"
case "${1:-tidy}" in
    "tidy"|"t")
        CLEAN_LEVEL="tidy"
        echo -e "${GREEN}Running 'tidy' cleanup...${NC}"
        echo "  - Removes CMakeFiles from source directories"
        echo "  - Removes log files and temporary artifacts"
        echo "  - Keeps compiled binaries"
        ;;
    "sweep"|"s")
        CLEAN_LEVEL="sweep"
        echo -e "${GREEN}Running 'sweep' cleanup...${NC}"
        echo "  - Aggressive cleanup of all artifacts"
        echo "  - Removes intermediate and temporary files"
        ;;
    "distclean"|"d")
        CLEAN_LEVEL="distclean"
        echo -e "${GREEN}Running 'distclean' cleanup...${NC}"
        echo "  - Removes entire build directory"
        echo "  - Project must be reconfigured after this"
        ;;
    "help"|"h")
        echo "Usage: $0 [tidy|sweep|distclean|help]"
        echo ""
        echo "Cleanup levels:"
        echo "  tidy (t)     - Light cleanup: remove CMakeFiles from source, logs, temps"
        echo "  sweep (s)    - Aggressive cleanup: all artifacts and intermediates"
        echo "  distclean (d)- Remove everything including build directory"
        echo "  help (h)     - Show this help message"
        echo ""
        echo "CMake targets (use with make):"
        echo "  make project-clean - Remove build artifacts (keep build dir)"
        echo "  make tidy         - Remove intermediate files, keep binaries"
        echo "  make sweep        - Aggressive cleanup of all artifacts"
        echo "  make distclean    - Remove entire build directory"
        echo "  make help-cleanup - Show this help message"
        echo ""
        echo "Examples:"
        echo "  $0 tidy      # Quick cleanup"
        echo "  $0 sweep     # Thorough cleanup"
        echo "  $0 distclean # Start fresh"
        exit 0
        ;;
    *)
        echo -e "${RED}Unknown cleanup level: $1${NC}"
        echo "Run '$0 help' for usage information."
        exit 1
        ;;
esac

echo ""
echo "Cleaning project root..."

# Common cleanup for all levels
remove_if_exists "src/CMakeFiles"
remove_if_exists "tests/CMakeFiles"
remove_if_exists "external/CMakeFiles"
remove_if_exists "CMakeCache.txt"
remove_if_exists "cmake_install.cmake"
remove_if_exists "Makefile"

# Remove CMakeFiles from subdirectories
for dir in src/*/; do
    remove_if_exists "${dir}CMakeFiles"
done

# Remove cmake_install.cmake and Makefile from src and tests
find_and_remove "src" "cmake_install.cmake" "cmake_install files"
find_and_remove "src" "Makefile" "Makefiles"
find_and_remove "tests" "cmake_install.cmake" "cmake_install files"
find_and_remove "tests" "Makefile" "Makefiles"

# Remove log files
find_and_remove "." "*.log" "log files"
find_and_remove "external" "*.log" "log files"
find_and_remove "." "latest" "artifact files"
find_and_remove "external" "latest" "artifact files"

# Level-specific cleanup
case "$CLEAN_LEVEL" in
    "tidy")
        remove_if_exists ".cmake"
        remove_if_exists "shaders"
        find_and_remove "." "*.zip" "zip archives"
        find_and_remove "external" "CMakeCache.txt" "CMake cache files"
        find_and_remove "external" "cmake_install.cmake" "cmake install files"
        find_and_remove "external" "Makefile" "Makefiles"
        remove_if_exists "external/build"
        ;;

    "sweep")
        remove_if_exists ".cmake"
        remove_if_exists "shaders"
        remove_if_exists "_deps"
        remove_if_exists "cmake-build-debug"
        find_and_remove "." "*.zip" "zip archives"
        find_and_remove "external" "CMakeCache.txt" "CMake cache files"
        find_and_remove "external" "cmake_install.cmake" "cmake install files"
        find_and_remove "external" "Makefile" "Makefiles"
        remove_if_exists "external/build"
        remove_if_exists "build/CMakeFiles"
        remove_if_exists "build/src/CMakeFiles"
        remove_if_exists "build/tests/CMakeFiles"
        remove_if_exists "build/external/CMakeFiles"
        remove_if_exists "build/_deps"
        ;;

    "distclean")
        remove_if_exists ".cmake"
        remove_if_exists "shaders"
        remove_if_exists "_deps"
        remove_if_exists "cmake-build-debug"
        remove_if_exists "build"
        remove_if_exists "external/build"
        find_and_remove "." "*.zip" "zip archives"
        ;;
esac

echo ""
echo -e "${GREEN}=== Cleanup Complete ===${NC}"
echo ""

# Show what's left
if [ "$CLEAN_LEVEL" != "distclean" ]; then
    echo "Project structure after cleanup:"
    ls -la | grep -v "^d" | grep -v "^total" | awk '{print "  " $9}' | grep -v "^  $" | head -10
    echo ""
fi

if [ "$CLEAN_LEVEL" = "distclean" ]; then
    echo -e "${YELLOW}Note: Project must be reconfigured with cmake before building.${NC}"
    echo ""
    echo "To rebuild:"
    echo "  mkdir build && cd build"
    echo "  cmake .."
    echo "  make -j\$(nproc)"
fi
