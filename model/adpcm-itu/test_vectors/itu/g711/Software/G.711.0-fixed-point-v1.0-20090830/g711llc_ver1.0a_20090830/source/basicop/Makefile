TARGET = basop32.o control.o count.o enh1632.o enh40.o oper_32b.o

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)

basop32.o: basop32.c stl.h
control.o: control.c stl.h
count.o: count.c stl.h
enh1632.o: enh1632.c stl.h
enh40.o: enh40.c stl.h
oper_32b.o: oper_32b.c oper_32b.h stl.h
stl.h: patch.h typedef.h basop32.h count.h move.h control.h enh1632.h enh40.h
enh1632.h: count.h
typedef.h: typedefs.h
basicop.h: stl.h oper_32b.h

