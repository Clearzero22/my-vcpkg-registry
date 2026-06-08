#!/bin/bash
# Quick setup script for private vcpkg registry

VCPKG_REGISTRY="/c/Users/admin/my-vcpkg-registry"

echo "🔧 Setting up private vcpkg registry..."
echo "Registry path: $VCPKG_REGISTRY"
echo ""

# Check if bashrc exists
BASHRC="$HOME/.bashrc"
if [ -f "$HOME/.zshrc" ]; then
    BASHRC="$HOME/.zshrc"
fi

# Add overlay ports to shell config
if ! grep -q "VCPKG_OVERLAY_PORTS" "$BASHRC" 2>/dev/null; then
    echo ""
    echo "# Private vcpkg registry" >> "$BASHRC"
    echo "export VCPKG_OVERLAY_PORTS=\"$VCPKG_REGISTRY/ports\"" >> "$BASHRC"
    echo "✅ Added VCPKG_OVERLAY_PORTS to $BASHRC"
else
    echo "⚠️  VCPKG_OVERLAY_PORTS already configured in $BASHRC"
fi

echo ""
echo "📦 Available packages:"
vcpkg search --overlay-ports="$VCPKG_REGISTRY/ports" 2>/dev/null || echo "No packages found yet"

echo ""
echo "✅ Setup complete!"
echo ""
echo "To use your private registry:"
echo "  1. Reload your shell: source ~/.bashrc"
echo "  2. Install package: vcpkg install example-package"
echo "  3. Or specify overlay: vcpkg install example-package --overlay-ports=$VCPKG_REGISTRY/ports"
echo ""
echo "See $VCPKG_REGISTRY/README.md for detailed documentation"