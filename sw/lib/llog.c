void
llog(d, dl, ds)
long	d, *dl, *ds;
{
	long	i;
	long	dqm, exp, mant;

	*ds = (d >> 15) ;
	dqm = *ds ? (65536 - d) & 32767 : d;

	for (i = 0 ; i <= 14; i++) {
		if ((dqm >> i) <= 1) {
			exp = i;
			break;
		}
	}

	mant = ((dqm << 7) >> exp) & 127;
	*dl = ((exp << 7) + mant) ;
}
