Electronic_Piano:
	sdcc -c Keypad4x4.c
	sdcc eOrgan-108321014.c Keypad4x4.rel
	packihx eOrgan-108321014.ihx > eOrgan-108321014.hex
	del *.asm *.ihx *.lk *.lst *.map *.mem *.rel *.rst *.sym