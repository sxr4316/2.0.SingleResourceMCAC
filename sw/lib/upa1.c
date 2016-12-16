long
upa1(pk01, pk11, a11, sigpk1)
long	pk01, pk11, a11, sigpk1;
{
	long	pks, uga1, a1s, ula1, ua1, a1t;

	pks = pk01 ^ pk11;
	if ((pks == 0) && (sigpk1 == 0))
		uga1 = 192;
	else if ((pks == 1) && (sigpk1 == 0))
		uga1 = 65344;
	else
		uga1 = 0;
	a1s = (a11 >> 15) ;
	ula1 = a1s ? (65536 - (( a11 >> 8) + 65280)) & 65535
	     : (65536 - (a11 >> 8)) & 65535;
	ua1 = (uga1 + ula1) & 65535;
	a1t = ( a11 + ua1) & 65535;
	return(a1t);
}
