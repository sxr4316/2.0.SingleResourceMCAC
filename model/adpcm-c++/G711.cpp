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

#include "common.h"
#include "G711.h"


/*
Members of class G711
*/


uint8_t G711::ALawEncode(int16_t pcm16)
	{
	int p = pcm16;
	unsigned a;  // A-law value we are forming
	if(p<0)
		{
		// -ve value
		// Note, ones compliment is used here as this keeps encoding symetrical
		// and equal spaced around zero cross-over, (it also matches the standard).
		p = ~p;
		a = 0x00; // sign = 0
		}
	else
		{
		// +ve value
		a = 0x80; // sign = 1
		}

	// Calculate segment and interval numbers
	p >>= 4;
	if(p>=0x20)
		{
		if(p>=0x100)
			{
			p >>= 4;
			a += 0x40;
			}
		if(p>=0x40)
			{
			p >>= 2;
			a += 0x20;
			}
		if(p>=0x20)
			{
			p >>= 1;
			a += 0x10;
			}
		}
	// a&0x70 now holds segment value and 'p' the interval number

	a += p;  // a now equal to encoded A-law value

	return a^0x55;	// A-law has alternate bits inverted for transmission
	}


int G711::ALawDecode(uint8_t alaw)
	{
	alaw ^= 0x55;  // A-law has alternate bits inverted for transmission

	unsigned sign = alaw&0x80;

	int linear = alaw&0x1f;
	linear <<= 4;
	linear += 8;  // Add a 'half' bit (0x08) to place PCM value in middle of range

	alaw &= 0x7f;
	if(alaw>=0x20)
		{
		linear |= 0x100;  // Put in MSB
		unsigned shift = (alaw>>4)-1;
		linear <<= shift;
		}

	if(!sign)
		return -linear;
	else
		return linear;
	}


uint8_t G711::ULawEncode(int16_t pcm16)
	{
	int p = pcm16;
	unsigned u;  // u-law value we are forming

	if(p<0)
		{
		// -ve value
		// Note, ones compliment is used here as this keeps encoding symetrical
		// and equal spaced around zero cross-over, (it also matches the standard).
		p = ~p;
		u = 0x80^0x10^0xff;  // Sign bit = 1, ^0x10 because this will get inverted later, ^0xff to invert final u-Law code
		}
	else
		{
		// +ve value
		u = 0x00^0x10^0xff;  // Sign bit = 0, ^0x10 because this will get inverted later, ^0xff to invert final u-Law code
		}

	p += 0x84; // Add uLaw bias

	if(p>0x7f00)
		p = 0x7f00;  // Clip to 15 bits

	// Calculate segment and interval numbers
	p >>= 3;	// Shift down to 13bit
	if(p>=0x100)
		{
		p >>= 4;
		u ^= 0x40;
		}
	if(p>=0x40)
		{
		p >>= 2;
		u ^= 0x20;
		}
	if(p>=0x20)
		{
		p >>= 1;
		u ^= 0x10;
		}
	// (u^0x10)&0x70 now equal to the segment value and 'p' the interval number ^0x10

	u ^= p; // u now equal to encoded u-law value (with all bits inverted)

	return u;
	}


int G711::ULawDecode(uint8_t ulaw)
	{
	ulaw ^= 0xff;  // u-law has all bits inverted for transmission

	int linear = ulaw&0x0f;
	linear <<= 3;
	linear |= 0x84;  // Set MSB (0x80) and a 'half' bit (0x04) to place PCM value in middle of range

	unsigned shift = ulaw>>4;
	shift &= 7;
	linear <<= shift;

	linear -= 0x84; // Subract uLaw bias

	if(ulaw&0x80)
		return -linear;
	else
		return linear;
	}


uint8_t G711::ALawToULaw(uint8_t alaw)
	{
	uint8_t sign=alaw&0x80;
	alaw ^= sign;
	alaw ^= 0x55;
	unsigned ulaw;
	if(alaw<45)
		{
		if(alaw<24)
			ulaw = (alaw<8) ? (alaw<<1)+1 : alaw+8;
		else
			ulaw = (alaw<32) ? (alaw>>1)+20 : alaw+4;
		}
	else
		{
		if(alaw<63)
			ulaw = (alaw<47) ? alaw+3 : alaw+2;
		else
			ulaw = (alaw<79) ? alaw+1 : alaw;
		}
	ulaw ^= sign;
	return ulaw^0x7f;
	}


uint8_t G711::ULawToALaw(uint8_t ulaw)
	{
	uint8_t sign=ulaw&0x80;
	ulaw ^= sign;
	ulaw ^= 0x7f;
	unsigned alaw;
	if(ulaw<48)
		{
		if(ulaw<=32)
			alaw = (ulaw<=15) ? ulaw>>1 : ulaw-8;
		else
			alaw = (ulaw<=35) ? (ulaw<<1)-40 : ulaw-4;
		}
	else
		{
		if(ulaw<=63)
			alaw = (ulaw==48) ? ulaw-3 : ulaw-2;
		else
			alaw = (ulaw<=79) ? ulaw-1 : ulaw;
		}
	alaw ^= sign;
	return alaw^0x55;
	}


unsigned G711::ALawEncode(uint8_t* dst, int16_t* src, size_t srcSize)
	{
	srcSize >>= 1;
	uint8_t* end = dst+srcSize;
	while(dst<end)
		*dst++ = ALawEncode(*src++);
	return srcSize;
	}


unsigned G711::ALawDecode(int16_t* dst, const uint8_t* src, size_t srcSize)
	{
	int16_t* end = dst+srcSize;
	while(dst<end)
		*dst++ = ALawDecode(*src++);
	return srcSize<<1;
	}


unsigned G711::ULawEncode(uint8_t* dst, int16_t* src, size_t srcSize)
	{
	srcSize >>= 1;
	uint8_t* end = dst+srcSize;
	while(dst<end)
		*dst++ = ULawEncode(*src++);
	return srcSize;
	}


unsigned G711::ULawDecode(int16_t* dst, const uint8_t* src, size_t srcSize)
	{
	int16_t* end = dst+srcSize;
	while(dst<end)
		*dst++ = ULawDecode(*src++);
	return srcSize<<1;
	}


unsigned G711::ALawToULaw(uint8_t* dst, const uint8_t* src, size_t srcSize)
	{
	uint8_t* end = dst+srcSize;
	while(dst<end)
		*dst++ = ALawToULaw(*src++);
	return srcSize;
	}


unsigned G711::ULawToALaw(uint8_t* dst, const uint8_t* src, size_t srcSize)
	{
	uint8_t* end = dst+srcSize;
	while(dst<end)
		*dst++ = ULawToALaw(*src++);
	return srcSize;
	}
