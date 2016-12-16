TARGET_COMMON = wmops_timer.o stack_profile.o
TARGET_ENCODER = g711llc_encode_file.o
TARGET_DECODER = g711llc_decode_file.o
TARGET = $(TARGET_COMMON) $(TARGET_ENCODER) $(TARGET_DECODER)

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)
