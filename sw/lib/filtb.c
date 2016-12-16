long
filtb(fi, dml1)
long	fi, dml1;
{
	long	dif, difs, difsx, dmlp1;

	dif = ((fi << 11) + 32768 - dml1) & 32767;
	difs = dif >> 14;

	difsx = difs ? (dif >> 7) + 16128 : dif >> 7;
	dmlp1 = (difsx + dml1) & 16383;
	return(dmlp1);
}
