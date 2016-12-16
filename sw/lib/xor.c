long
xor(dqn, dq)
long	dqn, dq;
{
	long	dqs, dqns, un;

	dqs = (dq >> 15);
	
	dqns = (dqn >> 10);
	un = (dqs ^ dqns);
	return(un);
}
