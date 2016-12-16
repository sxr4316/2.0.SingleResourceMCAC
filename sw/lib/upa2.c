long
upa2(pk01, pk11, pk21, a11, a21, sigpk1)
long	pk01, pk11, pk21, a11, a21, sigpk1;
{
	long	pks1, pks2, uga2a, a1s, fa1, fa, uga2s, uga2b, a2s, ula2, uga2, ua2, a2t;

	pks1 = pk01 ^ pk11;
	pks2 = pk01 ^ pk21;
	/* 114688 */
	uga2a = pks2 ? 114688 : 16384;
	a1s = (a11 >> 15) ;
	if (!a1s) 
		fa1 = (a11 >= 8192) ? 8191 << 2 : a11 << 2;
	else 
		fa1 = (a11 <= 57344) ? 24577 << 2 : (a11 << 2) & 131071;
	fa = pks1 ? fa1  : (131072 - fa1) & 131071;
	uga2b = (uga2a + fa) & 131071;
	uga2s = (uga2b >> 16) ;
	if ((uga2s == 0) && (sigpk1 == 0))
		uga2 = uga2b >> 7;
	else if ((uga2s == 1) && (sigpk1 == 0))
		uga2 = (uga2b >> 7) + 64512;
	else
		uga2 = 0;
	a2s = (a21 >> 15) ;
	ula2 = a2s ? (65536 - ((a21 >> 7) + 65024)) & 65535
	     : (65536 - (a21 >> 7)) & 65535;
	ua2 = (uga2 + ula2) & 65535;
	a2t = (a21 + ua2) & 65535;
	return(a2t);
}

