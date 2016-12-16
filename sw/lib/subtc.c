long
subtc(dmsp1, dmlp1, y1, tdp1)
long	dmsp1, dmlp1, y1, tdp1;
{
	long	dif, difs, difm, dthr, ax;

	dif = ((dmsp1 << 2) + 32768 - dmlp1) & 32767;
	difs = dif >> 14;

	difm = difs ? (32768 - dif) & 16383 : dif;
	dthr = dmlp1 >> 3;
	ax = ((y1 >= 1536) && (difm < dthr)) && (tdp1 == 0) ? 0 : 1;
	return(ax);
}
