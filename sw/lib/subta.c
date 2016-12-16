long
subta(sl, se1)
long	sl, se1;
{
	long	sls, sli, ses, sei, d;

	sls = sl >> 13;
	sli = sls ? 49152 + sl : sl;

	ses = se1 >> 14;
	sei = ses ? 32768 + se1 : se1;

	d = (sli + 65536 - sei) & 65535;
	return(d);
}
