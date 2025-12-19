CC = x86_64-linux-gnu-gcc
AS = x86_64-linux-gnu-as
LD = x86_64-linux-gnu-ld

GPPPARAMS = -m32 -Iinclude -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386 -z noexecstack

objects = obj/system/loader.o \
		obj/system/gdt.o \
		obj/system/memory.o \
		obj/system/load_idt.o \
		obj/system/sse.o \
		obj/tools/graphics.o \
		obj/fonts.mfp.o \
		obj/system/interrupts.o \
		obj/system/rtc.o \
		obj/drivers/keyboard.o \
		obj/drivers/mouse.o \
		obj/system/desktop.o \
		obj/tools/math.o \
		obj/drivers/serial.o \
		obj/tools/userlib.o \
		obj/drivers/ata.o \
		obj/drivers/pci.o \
		obj/system/storage.o \
		obj/system/exceptions.o \
		obj/system/acpi.o \
		obj/drivers/usb.o \
		obj/apps/debug.o \
		obj/apps/editor.o \
		obj/apps/files.o \
		obj/apps/info.o \
		obj/apps/runtime.o \
		obj/apps/runtime_session.o \
		obj/apps/image_viewer.o \
		obj/interpreter/interpreter.o \
		obj/interpreter/instructions.o \
        obj/system/kernel.o

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(GPPPARAMS) -c -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	$(AS) $(ASPARAMS) -o $@ $<

build: linker.ld $(objects)
	$(LD) $(LDPARAMS) -T $< -o $@ $(objects)
	@echo "Building FAT32 partition..."
	chmod +x create_disk_image.sh
	./create_disk_image.sh

build-iso: build
	@echo "Creating ISO (legacy method, no FAT32)..."
	tar -xzf i386-pc.tar.gz
	mkdir -p iso/boot/grub
	cp build iso/boot/build
	echo 'set timeout=0'					  > iso/boot/grub/grub.cfg
	echo 'set default=0'					 >> iso/boot/grub/grub.cfg
	echo ''								  >> iso/boot/grub/grub.cfg
	echo 'menuentry "MullowayK" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/build'	>> iso/boot/grub/grub.cfg
	echo '  boot'							>> iso/boot/grub/grub.cfg
	echo '}'								 >> iso/boot/grub/grub.cfg
	grub-mkrescue --directory=i386-pc --output=mullowayk.iso iso --locales=""
	rm -rf i386-pc iso

run:
	qemu-system-x86_64 -m 400 -serial stdio -hda mullowayk_disk.img

run-usb:
	qemu-system-x86_64 -drive file=mullowayk_disk.img,format=raw -m 400 -device piix3-usb-uhci,id=uhci -device usb-kbd,bus=uhci.0,port=1 -device usb-mouse,bus=uhci.0,port=2

run-iso:
	qemu-system-x86_64 -m 400 -serial stdio -cdrom mullowayk.iso

clean:
	rm -rf obj build mullowayk.iso mullowayk_disk.img disk_mount
