/* decode pcm to sign and mangnitude */
long
g711(pcm, aLaw)
long	pcm;
int	aLaw;
{
	long	q, s, sgn, mag, bias;

	if ( !aLaw ) {
		/* process u-law pcm */
		pcm ^= 0xff;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;
		bias = 33;

		mag = (((2 * q) + bias) << s) - bias;
		mag = (sgn ? mag | (1 << 13) : mag) & 16383;

	} else {
		/* process a-law pcm */
		pcm ^= 0x55;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;

		bias = (s > 0) ? 0x00021 : 0x00001;

		mag = (2 * q) + bias;
		mag = (s > 1) ? mag << (s - 1) : mag;
		mag = (sgn ? mag : mag | (1 << 12)) & 8191;
	}
	return(mag);
}
