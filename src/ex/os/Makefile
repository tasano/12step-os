PREFIX  = /usr/local
ARCH    = h8300-elf
BINDIR  = $(PREFIX)/bin
ADDNAME = $(ARCH)-

AR      = $(BINDIR)/$(ADDNAME)ar
AS      = $(BINDIR)/$(ADDNAME)as
CC      = $(BINDIR)/$(ADDNAME)gcc
LD      = $(BINDIR)/$(ADDNAME)ld
NM      = $(BINDIR)/$(ADDNAME)nm
OBJCOPY = $(BINDIR)/$(ADDNAME)objcopy
OBJDUMP = $(BINDIR)/$(ADDNAME)objdump
RANLIB  = $(BINDIR)/$(ADDNAME)ranlib
STRIP   = $(BINDIR)/$(ADDNAME)strip

OBJS    = startup.o main.o interrupt.o
OBJS    += lib.o serial.o timer.o rtl8019.o

# sources of kozos
OBJS    += kozos.o syscall.o memory.o command.o clock.o
OBJS    += ethernet.o
OBJS    += consdrv.o timerdrv.o netdrv.o

# for simulator
OBJS    += vector.o intr.o

TARGET  = kozos

CFLAGS  = -Wall -mh -nostdinc -nostdlib -fno-builtin
#CFLAGS += -mint32  # intをビットにすると掛算/割算ができなくなる
CFLAGS  += -I.
#CFLAGS += -g
CFLAGS  += -Os
#CFLAGS += -O0
CFLAGS  += -DKOZOS
#CFLAGS += -DSIMULATOR

LFLAGS  = -static -T ld.scr -L.

.SUFFIXES:  .c .o
.SUFFIXES:  .s .o
.SUFFIXES:  .S .o

all :			$(TARGET)

$(TARGET) :		$(OBJS)
				$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LFLAGS)
				cp $(TARGET) $(TARGET).elf
				$(STRIP) $(TARGET)

.c.o :			$<
				$(CC) -c $(CFLAGS) $<

.s.o :			$<
				$(CC) -c $(CFLAGS) $<

.S.o :			$<
				$(CC) -c $(CFLAGS) $<

clean :
				rm -f $(OBJS) $(TARGET) $(TARGET).elf
