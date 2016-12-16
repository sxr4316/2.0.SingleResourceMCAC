long
filta(fi, dms1)
long	fi, dms1;
{
	long	dif, difs, difsx, dmsp1;

	dif = ((fi << 9) + 8192 - dms1) & 8191;
	difs = dif >> 12;

	difsx = difs ? (dif >> 5) + 3840 : dif >> 5;
	dmsp1 = (difsx + dms1) & 4095;
	return(dmsp1);
}
