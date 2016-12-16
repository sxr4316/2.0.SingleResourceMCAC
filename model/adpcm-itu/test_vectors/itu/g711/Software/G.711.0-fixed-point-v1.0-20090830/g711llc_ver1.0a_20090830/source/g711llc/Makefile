TARGET_ENCODER = g711_itu.o output_bit_stream.o parcor.o g711llc_encoder.o G711Zencode_function.o tables.o window.o autocorr.o mapping.o fract_bits.o multirun.o
TARGET_DECODER = g711_itu.o input_bit_stream.o parcor.o g711llc_decoder.o G711Zdecode_function.o tables.o mapping.o fract_bits.o multirun.o
TARGET = $(TARGET_ENCODER) $(TARGET_DECODER)

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) $(TARGET)
