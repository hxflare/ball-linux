rm -f ball-linux.iso
cd isoroot/initramfs
find . | cpio -oH newc | gzip > ../initrd.img   
cd ../boot
mv ../initrd.img initrd.img
cd ../..
grub-mkrescue -o ball-linux.iso isoroot/
qemu-system-x86_64 -cdrom ball-linux.iso 