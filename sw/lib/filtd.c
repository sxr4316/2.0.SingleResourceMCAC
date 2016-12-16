long
filtd(wi, y1)
long	wi, y1;
{
	long	dif, difs, difsx, yut;

	dif = ((wi << 5) + 131072 - y1) & 131071;
	difs = dif >> 16;

	difsx = difs ? (dif >> 5) + 4096 : dif >> 5;
	yut = (y1 + difsx) & 8191;
	return(yut);
}
