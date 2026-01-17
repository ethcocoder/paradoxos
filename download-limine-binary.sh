#!/bin/bash

# Download and setup Limine bootloader binary files
echo "Setting up Limine bootloader for ParadoxOS..."

# Create limine directory if it doesn't exist
mkdir -p limine

# Try different Limine versions
VERSIONS=("v4.20231210.0" "v5.20240107.0" "v6.20240414.0")

for VERSION in "${VERSIONS[@]}"; do
    echo "Trying Limine $VERSION..."
    
    # Try to download the binary release
    if wget -q "https://github.com/limine-bootloader/limine/releases/download/$VERSION/limine-$VERSION.tar.xz"; then
        echo "Downloaded Limine $VERSION successfully"
        
        # Extract the archive
        tar -xf "limine-$VERSION.tar.xz"
        
        # Copy the bootloader files we need
        if [ -d "limine-$VERSION" ]; then
            cd "limine-$VERSION"
            
            # Build the bootloader
            make
            
            # Copy the required files
            if [ -f "limine-cd.bin" ]; then
                cp limine-cd.bin ../limine/
                cp limine-cd-efi.bin ../limine/
                cp limine.sys ../limine/
                cp limine ../limine/
                echo "✓ Limine bootloader files copied successfully"
                cd ..
                break
            fi
            cd ..
        fi
        
        # Clean up
        rm -f "limine-$VERSION.tar.xz"
        rm -rf "limine-$VERSION"
    fi
done

echo "Limine setup complete!"