/*
This program is distributed under the terms of the 'MIT license'. The text
of this licence follows...

Copyright (c) 2004 J.D.Medhurst (a.k.a. Tixy)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/**
@file

@brief Implementation of ITU-T (formerly CCITT) Recomendation %G711
*/

#ifndef G711_H
#define G711_H

/**
@defgroup g711 Audio Codec - ITU-T Recomendation G711
@{
*/

/**
@brief A class which implements ITU-T (formerly CCITT) Recomendation G711
	   "Pulse Code Modulation (PCM) of Voice Frequencies"

This encodes and decodes uniform PCM values to/from 8 bit A-law and u-Law values.

Note, the methods in this class use uniform PCM values which are of 16 bits precision,
these are 'left justified' values corresponding to the 13 and 14 bit values described
in G711.

@version 2006-05-20
	- Changed code to use standard typedefs, e.g. replaced uint8 with uint8_t, and made use of size_t.
*/
class G711
	{
public:
	/**
	Encode a single 16 bit uniform PCM value into an A-Law value

	@param pcm16 A 16 bit uniform PCM value

	@return 	 The A-Law encoded value corresponding to pcm16
	*/
	static uint8_t ALawEncode(int16_t pcm16);

	/**
	Decode a single A-Law value into a 16 bit uniform PCM value

	@param alaw An A-Law encoded value

	@return 	The 16 bit uniform PCM value corresponding to alaw
	*/
	static int ALawDecode(uint8_t alaw);

	/**
	Encode single 16 bit uniform PCM value into an u-Law value

	@param pcm16 A 16 bit uniform PCM value

	@return 	 The u-Law encoded value corresponding to pcm16
	*/
	static uint8_t ULawEncode(int16_t pcm16);

	/**
	Decode a single u-Law value into a 16 bit uniform PCM value

	@param ulaw An u-Law encoded value

	@return 	The 16 bit uniform PCM value corresponding to ulaw
	*/
	static int ULawDecode(uint8_t ulaw);

	/**
	Convert a single A-Law value into a u-law value.

	@param alaw An A-Law encoded value

	@return 	The u-law value.
	*/
	static uint8_t ALawToULaw(uint8_t alaw);

	/**
	Convert a single u-Law value into an A-law value.

	@param ulaw An u-Law encoded value

	@return 	The A-Law value
	*/
	static uint8_t ULawToALaw(uint8_t ulaw);

	/**
	Encode a buffer of 16 bit uniform PCM values into A-Law values

	@param dst	   Pointer to location to store A-Law encoded values
	@param src	   Pointer to the buffer of 16 bit uniform PCM values to be encoded
	@param srcSize The size, in bytes, of the buffer at \a src

	@return 	   The number of bytes which were stored at dst (equal to srcSize>>1)
	*/
	static unsigned ALawEncode(uint8_t* dst, int16_t* src, size_t srcSize);

	/**
	Decode a buffer of A-Law values into 16 bit uniform PCM values

	@param dst	   Pointer to location to store decoded 16 bit uniform PCM values
	@param src	   Pointer to the buffer of A-Law values to be decoded
	@param srcSize The size, in bytes, of the buffer at \a src

	@return 	   The number of bytes which were stored at \a dst (equal to srcSize<<1)
	*/
	static unsigned ALawDecode(int16_t* dst, const uint8_t* src, size_t srcSize);

	/**
	Encode a buffer of 16 bit uniform PCM values into u-Law values

	@param dst	   Pointer to location to store u-Law encoded values
	@param src	   Pointer to the buffer of 16 bit uniform PCM values to be encoded
	@param srcSize The size, in bytes, of the buffer at \a src

	@return 	   The number of bytes which were stored at \a dst (equal to srcSize>>1)
	*/
	static unsigned ULawEncode(uint8_t* dst, int16_t* src, size_t srcSize);

	/**
	Decode a buffer of u-Law values into 16 bit uniform PCM values

	@param dst	   Pointer to location to store decoded 16 bit uniform PCM values
	@param src	   Pointer to the buffer of u-Law values to be decoded
	@param srcSize The size, in bytes, of the buffer at \a src

	@return 	   The number of bytes which were stored at \a dst (equal to srcSize<<1)
	*/
	static unsigned ULawDecode(int16_t* dst, const uint8_t* src, size_t srcSize);

	/**
	Convert a buffer of A-Law values into u-law values.

	@param dst	   Pointer to location to store u-law values
	@param src	   Pointer to the buffer of A-Law values to be converted
	@param srcSize The size, in bytes, of the buffer at \a src

	@return 	   The number of bytes which were stored at \a dst (equal to srcSize)
	*/
	static unsigned ALawToULaw(uint8_t* dst, const uint8_t* src, size_t srcSize);

	/**
	Convert a buffer of u-Law values into A-law values.

	@param dst	   Pointer to location to store A-law values
	@param src	   Pointer to the buffer of u-Law values to be converted
	@param srcSize The size, in bytes, of the buffer at \a src

	@return 	   The number of bytes which were stored at \a dst (equal to srcSize)
	*/
	static unsigned ULawToALaw(uint8_t* dst, const uint8_t* src, size_t srcSize);
	};

/** @} */ // End of group

#endif

