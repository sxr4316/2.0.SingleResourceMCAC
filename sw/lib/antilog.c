long
antilog(dql, dqs)
long	dql, dqs;
{
	long	dq, dex, dmn, dqt, dqmag, ds;

	ds = (dql >> 11) ;
	dex = (dql >> 7) & 15;
	dmn = dql & 127;
	dqt = (1 << 7) + dmn;
	dqmag = ds ? 0 : (dqt << 7) >> ( 14 - dex);

	dq = ((dqs << 15) + dqmag);
	
	return(dq);
}
