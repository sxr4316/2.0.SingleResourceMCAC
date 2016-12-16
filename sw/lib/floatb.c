long
floatb(sri)
long	sri;
{
	long	srs, mag, exp, mant, sro, i;

	srs = (sri >> 15) ;
	mag = srs ? (65536 - sri) & 32767 : sri;

	exp = 0;
	for (i = 0; i <= 15; i += 1) {
		if (!(mag >> i)) {
			exp = i;
			break;
		}
	}
	mant = mag ? (mag << 6) >> exp : 1 << 5;
	sro = ((srs << 10) + ( exp << 6) + mant);
	return(sro);
}
