long
filte(yup1, yl1)
long	yup1;
long	yl1;
{
	long	dif, difs, difsx;
	long	ylp1;

	dif = ((yup1 + ((1048576 - yl1) >> 6)) & 16383);
	difs = dif >> 13;

	difsx = difs ? dif + 507904 : dif;
	ylp1 = (yl1 + difsx) & 524287;
	return(ylp1);
}
