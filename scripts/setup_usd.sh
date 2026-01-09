#!/bin/bash
#==============================================================================
# USD Vulkan Engine - OpenUSD Setup Script
#==============================================================================
#
# This script downloads and builds OpenUSD (Pixar USD) with proper
# versioned directory structure:
#   OpenUSD/src/<version>/    - Extracted source
#   OpenUSD/build/<version>/  - Build directory
#   OpenUSD/install/<version>/ - Installation directory
#
# Usage:
#   ./scripts/setup_usd.sh [VERSION]
#
# Arguments:
#   VERSION - OpenUSD version to build (default: v25.11)
#
# Environment:
#   HTTP_PROXY - Proxy server (auto-detected, can be overridden)
#   HTTPS_PROXY - Proxy server (auto-detected, can be overridden)
#
# To bypass proxy, set HTTP_PROXY="" and HTTPS_PROXY=""
#
# Proxy Auto-Detection:
#   Native Linux (Rocky Linux 9.7):   http://127.0.0.1:8118
#   WSL2 (Windows 11 Rocky Linux):    http://127.0.0.1:10808
#
#==============================================================================

set -e  # Exit on error

#==============================================================================
# Configuration
#==============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OPENUSD_DIR="$PROJECT_ROOT/OpenUSD"

# Default OpenUSD version
USD_VERSION="${1:-v25.11}"
# Remove 'v' prefix for directory names if present
VERSION_DIR="${USD_VERSION#v}"

# Detect WSL2
if grep -qi "microsoft" /proc/version 2>/dev/null; then
    DEFAULT_PROXY_PORT="10808"
    DETECTED_ENV="WSL2 (Windows 11)"
else
    DEFAULT_PROXY_PORT="8118"
    DETECTED_ENV="Native Linux"
fi

# Proxy configuration
if [ -z "$HTTP_PROXY" ] && [ -z "$HTTPS_PROXY" ]; then
    export HTTP_PROXY="http://127.0.0.1:${DEFAULT_PROXY_PORT}"
    export HTTPS_PROXY="http://127.0.0.1:${DEFAULT_PROXY_PORT}"
    export ALL_PROXY="$HTTPS_PROXY"
    AUTO_DETECTED_PROXY=1
else
    export ALL_PROXY="${HTTPS_PROXY:-$HTTP_PROXY}"
    AUTO_DETECTED_PROXY=0
fi

# Directory paths
SRC_DIR="$OPENUSD_DIR/src/$VERSION_DIR"
BUILD_DIR="$OPENUSD_DIR/build/$VERSION_DIR"
INSTALL_DIR="$OPENUSD_DIR/install/$VERSION_DIR"
ARCHIVE_FILE="$OPENUSD_DIR/src/OpenUSD-${USD_VERSION}.tar.gz"

# Download settings
DOWNLOAD_TIMEOUT=600
MAX_RETRIES=3
RETRY_DELAY=5

# Number of parallel build jobs
NPROC=$(nproc)

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

#==============================================================================
# Functions
#==============================================================================

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Download file with retry
download_file() {
    local url="$1"
    local output="$2"
    local description="$3"

    log_info "Downloading $description..."

    local attempt=1
    while [ $attempt -le $MAX_RETRIES ]; do
        if curl --proxy "$HTTP_PROXY" \
                --fail \
                --location \
                --retry $MAX_RETRIES \
                --retry-delay $RETRY_DELAY \
                --max-time $DOWNLOAD_TIMEOUT \
                --show-error \
                -o "$output" \
                "$url"; then
            local size=$(du -h "$output" | cut -f1)
            log_success "Downloaded: $description ($size)"
            return 0
        else
            log_warning "Attempt $attempt/$MAX_RETRIES failed for $description"
            if [ $attempt -lt $MAX_RETRIES ]; then
                log_info "Retrying in ${RETRY_DELAY}s..."
                sleep $RETRY_DELAY
            fi
            ((attempt++))
        fi
    done

    log_error "Failed to download: $description after $MAX_RETRIES attempts"
    return 1
}

#==============================================================================
# Main
#==============================================================================

main() {
    echo "=========================================="
    echo "USD Vulkan Engine - OpenUSD Setup"
    echo "=========================================="
    echo ""
    log_info "Environment: $DETECTED_ENV"
    if [ "$AUTO_DETECTED_PROXY" -eq 1 ]; then
        log_info "Proxy: $HTTP_PROXY (auto-detected)"
    else
        log_info "Proxy: $HTTP_PROXY (user-specified)"
    fi
    log_info "OpenUSD Version: $USD_VERSION"
    log_info "Project Root: $PROJECT_ROOT"
    echo ""

    # Test proxy connection
    log_info "Testing proxy connection..."
    if curl --proxy "$HTTP_PROXY" --max-time 10 -s -o /dev/null https://www.google.com; then
        log_success "Proxy connection OK"
    else
        log_warning "Proxy connection test failed (this may be OK if proxy is optional)"
    fi
    echo ""

    local TOTAL_STEPS=5
    local CURRENT_STEP=0

    #==========================================================================
    # Step 1: Create directory structure
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Creating Directories"
    log_info "=========================================="
    echo ""

    log_info "Directory structure:"
    log_info "  Source:   $SRC_DIR"
    log_info "  Build:    $BUILD_DIR"
    log_info "  Install:  $INSTALL_DIR"
    echo ""

    mkdir -p "$SRC_DIR"
    mkdir -p "$BUILD_DIR"
    mkdir -p "$OPENUSD_DIR/src"
    log_success "Directories created"
    echo ""

    #==========================================================================
    # Step 2: Download OpenUSD source
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Downloading OpenUSD"
    log_info "=========================================="
    echo ""

    if [ -d "$SRC_DIR" ] && [ -f "$SRC_DIR/CMakeLists.txt" ]; then
        log_info "Source already exists at $SRC_DIR, skipping download..."
    else
        # Clean up any existing partial extraction
        rm -rf "$SRC_DIR"

        if [ -f "$ARCHIVE_FILE" ]; then
            log_info "Archive already exists, extracting..."
        else
            download_file \
                "https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/${USD_VERSION}.tar.gz" \
                "$ARCHIVE_FILE" \
                "OpenUSD ${USD_VERSION}"
        fi

        log_info "Extracting OpenUSD source..."
        mkdir -p "$SRC_DIR"
        tar -xzf "$ARCHIVE_FILE" -C "$SRC_DIR" --strip-components=1
        log_success "Extracted to $SRC_DIR"
    fi
    echo ""

    #==========================================================================
    # Step 3: Configure CMake
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Configuring CMake"
    log_info "=========================================="
    echo ""

    log_info "CMake options:"
    log_info "  CMAKE_BUILD_TYPE=Release"
    log_info "  PXR_BUILD_USD_USDA=OFF            (Disable usda authoring tool)"
    log_info "  PXR_BUILD_USD_USDC=ON             (Enable usdc compiler)"
    log_info "  PXR_ENABLE_PYTHON_SUPPORT=OFF     (Disable Python support)"
    log_info "  PXR_BUILD_IMAGING=OFF             (Disable imaging components)"
    log_info "  PXR_BUILD_USDVIEW=OFF             (Disable usdview)"
    log_info "  PXR_BUILD_USDRENDERER=OFF        (Disable Hydra renderer)"
    log_info "  PXR_BUILD_TESTS=OFF               (Disable tests)"
    log_info "  PXR_BUILD_EXAMPLES=OFF            (Disable examples)"
    log_info "  PXR_BUILD_MATERIALX_ADAPTER_PLUGIN=OFF"
    log_info "  CMAKE_INSTALL_PREFIX=$INSTALL_DIR"
    echo ""

    cd "$BUILD_DIR"

    cmake "$SRC_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DPXR_BUILD_USD_USDA=OFF \
        -DPXR_BUILD_USD_USDC=ON \
        -DPXR_ENABLE_PYTHON_SUPPORT=OFF \
        -DPXR_BUILD_IMAGING=OFF \
        -DPXR_BUILD_USDVIEW=OFF \
        -DPXR_BUILD_USDRENDERER=OFF \
        -DPXR_BUILD_TESTS=OFF \
        -DPXR_BUILD_EXAMPLES=OFF \
        -DPXR_BUILD_MATERIALX_ADAPTER_PLUGIN=OFF \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"

    log_success "CMake configuration complete"
    echo ""

    #==========================================================================
    # Step 4: Build OpenUSD
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Building OpenUSD"
    log_info "=========================================="
    echo ""

    log_info "Building with $NPROC parallel jobs..."
    echo ""

    make -j"$NPROC"

    log_success "Build complete"
    echo ""

    #==========================================================================
    # Step 5: Install OpenUSD
    #==========================================================================
    ((CURRENT_STEP = CURRENT_STEP + 1)) || true
    log_info "=========================================="
    log_info "Step $CURRENT_STEP/$TOTAL_STEPS: Installing OpenUSD"
    log_info "=========================================="
    echo ""

    make install

    log_success "Installation complete"
    echo ""

    #==========================================================================
    # Summary
    #==========================================================================
    log_info "=========================================="
    log_info "Setup Summary"
    log_info "=========================================="
    echo ""

    log_success "OpenUSD ${USD_VERSION} built and installed successfully!"
    echo ""

    log_info "Installation details:"
    log_info "  Version:  $USD_VERSION"
    log_info "  Source:   $SRC_DIR"
    log_info "  Build:    $BUILD_DIR"
    log_info "  Install:  $INSTALL_DIR"
    echo ""

    log_info "Installed files:"
    log_info "  Headers:  $INSTALL_DIR/include"
    log_info "  Libraries: $INSTALL_DIR/lib"
    log_info "  Plugins:  $INSTALL_DIR/lib/usd"
    echo ""

    # Verify installation
    if [ -f "$INSTALL_DIR/lib/libusd.so" ] || [ -f "$INSTALL_DIR/lib/libusd.a" ]; then
        log_success "USD libraries verified!"
    else
        log_warning "Warning: USD libraries not found at expected location"
    fi

    # Create symlink for backward compatibility (pointing to versioned install)
    if [ -L "$PROJECT_ROOT/OpenUSD/install/lib" ]; then
        log_info "Symlink already exists"
    else
        mkdir -p "$PROJECT_ROOT/OpenUSD/install"
        ln -sf "$INSTALL_DIR/lib" "$PROJECT_ROOT/OpenUSD/install/lib"
        ln -sf "$INSTALL_DIR/include" "$PROJECT_ROOT/OpenUSD/install/include"
        log_success "Created symlinks for backward compatibility"
    fi
    echo ""

    log_info "Next steps:"
    log_info "  1. Run: HTTP_PROXY=$HTTP_PROXY HTTPS_PROXY=$HTTPS_PROXY cmake -B build -DCMAKE_BUILD_TYPE=Release"
    log_info "  2. Run: cd build && make -j\$(nproc)"
    echo ""
}

# Run main
main "$@"
