#!/bin/bash
# install.sh - Install CHARM system components

set -e

# Default installation directory
INSTALL_DIR="/usr/local"
PREFIX=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --prefix=*)
            PREFIX="${1#*=}"
            shift
            ;;
        --help)
            echo "Usage: $0 [--prefix=DIR]"
            echo "  --prefix=DIR    Install files in PREFIX directory"
            echo "  --help          Display this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Try '$0 --help' for more information."
            exit 1
            ;;
    esac
done

if [ -n "$PREFIX" ]; then
    INSTALL_DIR="$PREFIX"
fi

echo "Installing CHARM system to $INSTALL_DIR"

# Create directories
mkdir -p "$INSTALL_DIR/bin"
mkdir -p "$INSTALL_DIR/lib"
mkdir -p "$INSTALL_DIR/include/charm"
mkdir -p "$INSTALL_DIR/share/charm/docs"

# Install binary
if [ -f "charm" ]; then
    cp charm "$INSTALL_DIR/bin/"
    chmod 755 "$INSTALL_DIR/bin/charm"
    echo "Installed charm binary to $INSTALL_DIR/bin/"
else
    echo "Error: charm binary not found. Run 'make' first."
    exit 1
fi

# Install library
if [ -f "libcharm.a" ]; then
    cp libcharm.a "$INSTALL_DIR/lib/"
    chmod 644 "$INSTALL_DIR/lib/libcharm.a"
    echo "Installed libcharm.a to $INSTALL_DIR/lib/"
else
    echo "Error: libcharm.a not found. Run 'make' first."
    exit 1
fi

# Install headers
cp include/*.h "$INSTALL_DIR/include/charm/"
chmod 644 "$INSTALL_DIR/include/charm/"*.h
echo "Installed header files to $INSTALL_DIR/include/charm/"

# Install documentation
cp docs/*.md "$INSTALL_DIR/share/charm/docs/"
chmod 644 "$INSTALL_DIR/share/charm/docs/"*.md
echo "Installed documentation to $INSTALL_DIR/share/charm/docs/"

# Create log directory with appropriate permissions
mkdir -p "$INSTALL_DIR/var/log/charm"
chmod 755 "$INSTALL_DIR/var/log/charm"
echo "Created log directory at $INSTALL_DIR/var/log/charm/"

echo "Installation complete!"
echo ""
echo "To use CHARM, run: $INSTALL_DIR/bin/charm"
echo "For development, use: -I$INSTALL_DIR/include/charm -L$INSTALL_DIR/lib -lcharm"
