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

@brief Implementation of ITU-T (formerly CCITT) Recomendation %G726
*/

#ifndef G726_H
#define G726_H

/**
@defgroup g726 Audio Codec - ITU-T Recomendation G726
@{
*/

/**
@brief A class which implements ITU-T (formerly CCITT) Recomendation G726
	   "40, 32, 24, 16 kbit/s Adaptive Differential Pulse Code Modulation (ADPCM)"

G726 replaces recomendations G721 and G723.

Note, this implemetation reproduces bugs found in the G191 reference implementation
of %G726. These bugs can be controlled by the #IMPLEMENT_G191_BUGS macro.

@see IMPLEMENT_G191_BUGS

@version 2005-02-13
	- Added constructor G726(),
	- Added Encode(void* dst, int dstOffset, const void* src, size_t srcSize) and
	- Added Decode(void* dst, const void* src, int srcOffset, unsigned srcSize);

@version 2006-05-20
	- Changed code to use standard typedefs, e.g. replaced uint8 with uint8_t, and made use of size_t.

@version 2007-01-13
	- Fixed GCC compilation warnings.
*/
class G726
	{
public:
	/**
	Enumeration used to specify the coding law used for encoder input and decoder output.
	*/
	enum Law{
		uLaw=0, 	/**< u law */
		ALaw=1, 	/**< A law */
		PCM16=2 	/**< 16 bit uniform PCM values. These are 'left justified' values
						 corresponding to the 14 bit values in G726 Annex A. */
		};

	/**
	Enumeration used to specify the ADPCM bit-rate.
	*/
	enum Rate{
		Rate16kBits=2,	/**< 16k bits per second (2 bits per ADPCM sample) */
		Rate24kBits=3,	/**< 24k bits per second (3 bits per ADPCM sample) */
		Rate32kBits=4,	/**< 32k bits per second (4 bits per ADPCM sample) */
		Rate40kBits=5	/**< 40k bits per second (5 bits per ADPCM sample) */
		};

	/**
	Contructor which initialises the object to the reset state.
	@see Reset()
	@since 2005-02-13
	*/
	inline G726()
		{
		Reset();
		}

	/**
	Clears the internal state variables to their 'reset' values.
	Call this function before starting to decode/encode a new audio stream.
	*/
	void Reset();

	/**
	Set the encoding law used for encoder input and decoder output.

	@param law The encoding law.
	*/
	void SetLaw(Law law);

	/**
	Set the ADPCM bit-rate used for encoder output and decoder intput.

	@param rate The ADPCM bit-rate.
	*/
	void SetRate(Rate rate);

	/**
	Encodes a single PCM value as an ADPCM value.

	@pre		The coding law and ADPCM bit-rate should have been previous initialised
				with SetLaw() and SetRate().

	@param pcm	The PCM value to encode. The bits in this value which lie outside
				the range of values for the currently set encoding law, are ignored.

	@return 	The encoded ADPCM value. The number of significant bits in this value
				are dependant on the bit-rate set by SetRate(). Unused bits are set to zero.
	*/
	unsigned Encode(unsigned pcm);

	/**
	Decodes a single ADPCM value into a PCM value.

	@pre		 The coding law and ADPCM bit-rate should have been previous initialised
				 with SetLaw() and SetRate().

	@param adpcm The ADPCM value to decode. The bits in this value which lie outside
				 the range of values for the currently set bit-rate, are ignored.

	@return 	 The decoded PCM value. This is in the format specified by SetLaw().

	@see IMPLEMENT_G191_BUGS
	*/
	unsigned Decode(unsigned adpcm);

	/**
	Encode a buffer of uniform PCM values into ADPCM values.

	Each ADPCM value only occupies the minimum number of bits required and successive
	values occupy adjacent bit positions. E.g. Four 3 bit ADPCM values (A,B,C,D) are
	stored in two successive bytes like this: 1st byte: ccbbbaaa 2nd byte: ----dddc.
	Note that any unused bits in the last byte are set to zero.

	@pre		The coding law and ADPCM bit-rate should have been previous initialised
				with SetLaw() and SetRate().

	@param dst		 Pointer to location to store ADPCM values.
	@param dstOffset Offset from \a dst, in number-of-bits, at which the decoded values
					 will be stored. I.e. the least significant bit of the first ADPCM
					 value will be stored in byte
					 @code	 dst[dstOffset>>3]	 @endcode
					 at bit position
					 @code	 dstOffset&7		 @endcode
					 Where the bit 0 is the least significant bit in a byte
					 and bit 7 is the most significant bit.
	@param src		 Pointer to the buffer of PCM values to be converted.
	@param srcSize	 The size in bytes of the buffer at \a src.
					 Must be a multiple of the size of a single PCM sample.

	@return 		 The number of bits were stored in the \a dst buffer.

	@since 2005-02-13
	*/
	unsigned Encode(void* dst, int dstOffset, const void* src, size_t srcSize);

	/**
	Decode a buffer of ADPCM values into uniform PCM values.

	Each ADPCM value only occupies the minimum number of bits required and successive
	values occupy adjacent bit positions. E.g. Four 3 bit ADPCM values (A,B,C,D) are
	stored in two successive bytes like this: 1st byte: ccbbbaaa 2nd byte: ----dddc.

	@pre		The coding law and ADPCM bit-rate should have been previous initialised
				with SetLaw() and SetRate().

	@param dst		 Pointer to location to store PCM values.
	@param src		 Pointer to the buffer of ADPCM values to be converted.
	@param srcOffset Offset from \a src, in number-of-bits, from which the ADPCM values
					 will be read. I.e. the least significant bit of the first ADPCM
					 value will be read from byte
					 @code	 src[srcOffset>>3]	 @endcode
					 at bit position
					 @code	 srcOffset&7		 @endcode
					 Where the bit 0 is the least significant bit in a byte
					 and bit 7 is the most significant bit.
	@param srcSize	 The number of bits to be read from the buffer at \a src.
					 Must be a multiple of the size of a single ADPCM value.

	@return 		 The number of bytes which were stored in the \a dst buffer.

	@see IMPLEMENT_G191_BUGS

	@since 2005-02-13
	*/
	unsigned Decode(void* dst, const void* src, int srcOffset, unsigned srcSize);
private:
	void InputPCMFormatConversionAndDifferenceSignalComputation(unsigned S,int SE,int& D);
	void AdaptiveQuantizer(int D,unsigned Y,unsigned& I);
	void InverseAdaptiveQuantizer(unsigned I,unsigned Y,unsigned& DQ);
	void QuantizerScaleFactorAdaptation1(unsigned AL,unsigned& Y);
	void QuantizerScaleFactorAdaptation2(unsigned I,unsigned Y);
	void AdaptationSpeedControl1(unsigned& AL);
	void AdaptationSpeedControl2(unsigned I,unsigned y,unsigned TDP,unsigned TR);
	void AdaptativePredictorAndReconstructedSignalCalculator1(int& SE,int& SEZ);
	void AdaptativePredictorAndReconstructedSignalCalculator2(unsigned DQ,unsigned TR,int SE,int SEZ,int& SR,int& A2P);
	void ToneAndTransitionDetector1(unsigned DQ,unsigned& TR);
	void ToneAndTransitionDetector2(int A2P,unsigned TR,unsigned& TDP);
	void OutputPCMFormatConversionAndSynchronousCodingAdjustment(int SR,int SE,unsigned Y,unsigned I,unsigned& SD);
	void DifferenceSignalComputation(int SL,int SE,int& D);
	void OutputLimiting(int SR,int& S0);
	unsigned EncodeDecode(unsigned input,bool encode);
private:
	Law LAW;
	Rate RATE;
	// Persistant state for DELAY elements...
	int A1;
	int A2;
	unsigned AP;
	int Bn[6];
	unsigned DML;
	unsigned DMS;
	unsigned DQn[6];
	int PK1;
	int PK2;
	unsigned SR1;
	unsigned SR2;
	unsigned TD;
	unsigned YL;
	unsigned YU;

	friend class G726Test;
	};

/**
When the source code is compiled with this macro defined, the code will reproduce
bugs found in the G191 reference implementation of %G726. These bugs are also required
so that the test sequences supplied in %G726 Appendix II produce the 'correct' results.

The bugs affect the G726::Decode functions when the coding is uLaw or ALaw.

@since 2005-02-13
*/
#define IMPLEMENT_G191_BUGS

/** @} */ // End of group

#endif

