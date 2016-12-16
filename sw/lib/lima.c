long
lima(ap1)
long	ap1;
{
	long	al1;

	al1 = (ap1 >= 256) ? 64 : ap1 >> 2;
	return(al1);
}

