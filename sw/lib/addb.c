long
addb(dq, se1)
long	dq, se1;
{
	long	dqs, dqi, ses, sei, sri;

		dqs = dq >> 15;
		dqi = dqs ? (65536 - (dq & 32767)) & 65535 : dq;
	
	ses = se1 >> 14;
	sei = ses ? (1 << 15) + se1 : se1;
	sri = (dqi + sei) & 65535;
	return(sri);
}
