long
addc(dq, sez1, sigpk1)
//added br = bit rate 
long	dq, sez1, *sigpk1;
{
	long	dqs, dqi, sezs, sezi, pko, dqsez;

	
	dqs = (dq >> 15);
	dqi = dqs ? (65536 - (dq & 32767)) & 65535 : dq;	

	sezs = sez1 >> 14;
	sezi = sezs ? (1 << 15) + sez1 : sez1;
	dqsez = (dqi + sezi) & 65535;
	pko = (dqsez >> 15);
	*sigpk1 = dqsez == 0 ? 1 : 0;
	return(pko);
}
