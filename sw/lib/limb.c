long
limb(yut)
long	yut;
{
	long	yup1, geul, gell;

	geul = (((yut + 11264) & 16383) >> 13) ;
	gell = ((yut + 15840) & 16383) >> 13;

	if (gell == 1) 
		yup1 = 544;
	else if (geul == 0) 
		yup1 = 5120;
	else 
		yup1 = yut;

	return(yup1);
}

