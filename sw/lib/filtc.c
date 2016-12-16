long
filtc(ax, ap1)
long	ax, ap1;
{
	long	dif, difs, difsx, app1;

	dif = ((ax << 9) + 2048 - ap1) & 2047;
	difs = dif >> 10;

	difsx = difs ? (dif >> 4) + 896 : dif >> 4;
	app1 = (difsx + ap1) & 1023;
	return(app1);
}
