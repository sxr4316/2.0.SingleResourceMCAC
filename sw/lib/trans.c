long
trans(td1, yl1, dq)
long	td1, yl1, dq;
{
	long	tr, dqmag, ylint, ylfrac, thr1, thr2, dqthr;
	dqmag = dq & 32767;
	ylint = (yl1 >> 15) ;	
	ylfrac = (yl1 >> 10) & 31;
	thr1 = (32 + ylfrac) << ylint;

	thr2 = ylint > 9 ? 31 << 10 : thr1;
		

	dqthr = (thr2 + (thr2 >> 1)) >> 1;
	tr = (dqmag > dqthr) && (td1 == 1) ? 1 : 0;

	return(tr);
}

