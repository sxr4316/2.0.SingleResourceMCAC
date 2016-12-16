long
floata(dq)
long	dq;
{
	long	dqs, mag, exp, mant, dqo;
	long	i;


	dqs = (dq >> 15) ;
	mag = dq & 32767;

	exp = 0;
	for (i = 0; i <= 15; i += 1) {

		if (!(mag >> i)) {
			exp = i;
			break;
		}
	}
	mant = mag ? (mag << 6) >> exp : 1 << 5;
	dqo = ((dqs << 10) + ( exp << 6) + mant);
	return(dqo);
}
