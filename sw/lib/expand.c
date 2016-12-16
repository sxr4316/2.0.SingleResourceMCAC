long
expand(ss, aLaw)
long	ss;
int	aLaw;
{
	long	sss, ssm, ssq, sl;

	if ( !aLaw ) {
		/* process u-law */
		sss = ss >> 13;
		ssq = ss & 8191;
	} else {
		/* process a-law */
		sss = ss >> 12;
		ssm = ss & 4095;
		ssq = ssm << 1;
	}
	sl = sss ? (16384 - ssq) & 16383 : ssq;
	return(sl);
}
