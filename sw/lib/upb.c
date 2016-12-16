long
upb(un, bn, dq, br)
long	un, bn, dq, br;
{
	long	ugbn, bns, ulbn, ubn, bnp, dqmag;
	dqmag = dq & 32767;

	if ((un == 0) && (dqmag != 0))
		ugbn = 128;
	else if (un && (dqmag != 0))
		ugbn = 65408 ;
	else
		ugbn = 0;	
	bns = bn >> 15;
	if (br < 40)
	{
		ulbn = bns ? (65536 - ((bn >> 8) + 65280)) & 65535 
	    	 : (65536 - (bn >> 8)) & 65535;
	}
	else
	{
		ulbn = bns ? (65536 - ((bn >> 9) + 65408)) & 65535 
	    	 : (65536 - (bn >> 9)) & 65535;
	}

	ubn = (ugbn + ulbn) & 65535;
	bnp = (bn + ubn) & 65535;
	return(bnp);
}

