void
accum(wa1, wa2, wb1, wb2, wb3, wb4, wb5, wb6, sez, se)
long	wa1, wa2, wb1, wb2, wb3, wb4, wb5, wb6, *sez, *se;
{
	long	sezi, sei;

	sezi = (((((((((wb1 + wb2) & 65535) + wb3) & 65535) + wb4) & 65535)
	     + wb5) & 65535) + wb6) & 65535;
	sei = (((sezi + wa2) & 65535) + wa1) & 65535;
	*sez = (sezi >> 1) ;
	*se = (sei >> 1) ;
}
