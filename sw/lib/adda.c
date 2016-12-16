
long
adda(dqln, y1)
long	dqln, y1;
{
	long	dql;

	dql = (dqln + (y1 >> 2)) & 4095;
	return(dql);
}

