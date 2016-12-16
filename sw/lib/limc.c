#define A2UL	12288
#define A2LL	53248

long
limc(a2t)
long	a2t;
{
	long	a2p1;

	if ((a2t >= 32768) && (a2t <= A2LL)) 
		a2p1 = A2LL;
	else if ((a2t >= A2UL) && (a2t <= 32767)) 
		a2p1 = A2UL;
	else 
		a2p1 = a2t;
	//a2p1 &= 65535;
	return(a2p1);
}


