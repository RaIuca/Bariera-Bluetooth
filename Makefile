NAME    	= Bluetooth
MCU     	= atmega328p
F_CPU   	= 16000000
INCDIR		= inc
SRCDIR		= src/
OBJDIR    	= bin
TARGET		=${addprefix ${OBJDIR},/${NAME}}
CC      	= toolchain/bin/avr-gcc.exe
OBJCOPY 	= toolchain/bin/avr-objcopy.exe
AVRDUDE		= toolchain/prog/avrdude.exe
CFLAGS  	= 	-std=c99 			\
			-Wall 				\
			-Wextra 			\
			-g 				\
			-Os 				\
			-mmcu=${MCU}			\
			-DF_CPU=${F_CPU}	
SRCS		= 	app.c				\
			iic.c				\
			rtc.c				\
			timer.c				\
			usart.c		

compile:

	@echo **************************************************
	@echo *	Compileaza...
	@${CC} ${CFLAGS} -o ${TARGET}.bin ${addprefix ${SRCDIR},${SRCS}} -I ${INCDIR}
	@echo **************************************************
	@echo *	Extrage fisier hex...
	@${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex
	@echo **************************************************
	@echo *	Terminat						
	@echo **************************************************
flash:
	@echo **************************************************
	@echo *	Scrie programul in controller...
	@${AVRDUDE} -p ${MCU} -c usbasp -U flash:w:${TARGET}.hex:i -F -P usb

re:	$(clear) clean compile

clean:
	@echo **************************************************
	@echo *	Sterge fisier obiecte...
	@del  /F /Q /A $(OBJDIR)

qflash: clean compile flash
