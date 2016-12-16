long
mix(al1, yu1, yl1)
long	al1, yu1;
long	yl1;
{
	long	dif, difs, difm, prodm, prod, y1;

	dif = (yu1 + 16384 - (yl1 >> 6)) & 16383;
	difs = dif >> 13;

	difm = difs ? (16384 - dif) & 8191 : dif;
	prodm = (difm * al1) >> 6;
	prod = difs ? (16384 - prodm) & 16383 : prodm;
	y1 = ((yl1 >> 6) + prod) & 8191;

	return(y1);
}

