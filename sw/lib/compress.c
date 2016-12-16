extern long inv_g711();

long
compress(sri, aLaw, br)
long	sri, br;
int	aLaw;
{
	long	is, im, imag, sp;

	is = (sri >> 15);
	im = is ? ((65536 - sri) & 32767) : (sri);

	if ( !aLaw )
	{
		/* process u-pcm */
		imag = im;
	}
	else
	{
		/* process a-pcm */
		if (br == 40)
		{
			im = ( sri == 0x8000) ? 2 : im;
		}
		else
		{
			im=(sri > 65535) ? 2 : im;
		}

		imag = is ? ((im + 1) >> 1) : (im >> 1);
		if(is)
			--imag;
		if (imag > 4095)
			imag = 4095;
	}

	sp = inv_g711(is, imag, aLaw);

	sp = sp & 0xFF;
	return(sp);
}
