long
subtb(dl, y1)
long	dl, y1;
{
	long	dln;

	dln = (dl + 4096 - (y1 >> 2)) & 4095;
	return(dln);
}

