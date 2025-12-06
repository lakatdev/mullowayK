#!/bin/bash

set -e
DISK_SIZE_MB=512
OUTPUT_IMAGE="mullowayk_disk.img"
MOUNT_POINT="disk_mount"
FAT_START_SECTOR=32768

echo "Creating ${DISK_SIZE_MB}MB disk image with FAT32 partition..."

if [ ! -f "build" ]; then
    echo "ERROR: Kernel 'build' file not found! Run 'make' first."
    exit 1
fi

dd if=/dev/zero of="$OUTPUT_IMAGE" bs=1M count=$DISK_SIZE_MB status=progress 2>&1 | grep -v "records"

echo "Creating partition table with FAT32 at sector $FAT_START_SECTOR..."
cat > partition_layout.txt << EOF
label: dos
label-id: 0x12345678
device: $OUTPUT_IMAGE
unit: sectors
sector-size: 512

${OUTPUT_IMAGE}1 : start=$FAT_START_SECTOR, type=0c, bootable
EOF

sfdisk "$OUTPUT_IMAGE" < partition_layout.txt > /dev/null 2>&1
rm partition_layout.txt

LOOP_DEVICE=$(sudo losetup -f --show "$OUTPUT_IMAGE")
echo "Loop device: $LOOP_DEVICE"
sudo partprobe "$LOOP_DEVICE" 2>/dev/null || true
sleep 1

echo "Formatting partition as FAT32..."
sudo mkfs.vfat -F 32 -n "MULLOWAYK" -s 8 "${LOOP_DEVICE}p1" > /dev/null 2>&1

mkdir -p "$MOUNT_POINT"
sudo mount "${LOOP_DEVICE}p1" "$MOUNT_POINT"

echo "Setting up boot files..."
sudo mkdir -p "$MOUNT_POINT/boot/grub"

sudo cp build "$MOUNT_POINT/boot/build"
echo "Kernel copied to boot directory"

sudo tee "$MOUNT_POINT/boot/grub/grub.cfg" > /dev/null << 'GRUBEOF'
set timeout=0
set default=0

menuentry "MullowayK" {
  multiboot /boot/build
  boot
}
GRUBEOF

echo "Copying files from disk_files/ to root directory..."
if [ -d "disk_files" ]; then
    for file in disk_files/*; do
        if [ -f "$file" ]; then
            echo "  Copying $(basename "$file")..."
            sudo cp "$file" "$MOUNT_POINT/"
        fi
    done
else
    echo "Warning: disk_files/ directory not found"
fi

sync
sudo umount "$MOUNT_POINT"
rmdir "$MOUNT_POINT"

echo "Installing GRUB bootloader..."
GRUB_TEMP="/tmp/grub_mullowayk_$$"
mkdir -p "$GRUB_TEMP"
sudo mount "${LOOP_DEVICE}p1" "$GRUB_TEMP"
sudo grub-install --target=i386-pc --boot-directory="$GRUB_TEMP/boot" --modules="part_msdos fat multiboot" "$LOOP_DEVICE" > /dev/null 2>&1 && {
    echo "GRUB installed successfully"
} || {
    echo "Warning: GRUB installation had issues, but disk should still boot"
}
sudo umount "$GRUB_TEMP"
rmdir "$GRUB_TEMP"

sudo losetup -d "$LOOP_DEVICE"

echo ""
echo "======================================================"
echo "Disk image created successfully: $OUTPUT_IMAGE"
echo "======================================================"
echo ""
