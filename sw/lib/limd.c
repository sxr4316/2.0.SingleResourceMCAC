#define OME	15360

long
limd(a1t, a2p1)
long	a1t, a2p1;
{
	long	a1ul, a1ll, a1p1;

	a1ul = (OME + 65536 - a2p1) & 65535;
	a1ll = (a2p1 + 65536 - OME) & 65535;

	if ((a1t >= 32768) && (a1t <= a1ll)) 
		a1p1 = a1ll;
	else if ((a1t >= a1ul) && (a1t <= 32767)) 
		a1p1 = a1ul;
	else 
		a1p1 = a1t;
	return(a1p1);
}
