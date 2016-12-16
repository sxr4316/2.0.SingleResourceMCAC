long
tone(a2p)
long	a2p;
{
	long det = 0 ;

	if ((32768 <= a2p) && (a2p < 53760)) 
		det = 1;

	return(det);
}

