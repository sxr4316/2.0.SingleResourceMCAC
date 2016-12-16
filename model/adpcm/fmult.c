
#include <stdio.h>
#include <stdlib.h>

long random () ;
long fmult(long x, long y1, int test) ;

main ()
{
long	wa, x, y1;
int i, test ;

test = 0 ;

for (i = 0 ; i < 20 ; i ++)
{
	x = random() & 0xffff ;
	y1 = random() & 0x7ff ;
	if (test)
		printf ("x = 0x%x, y1 = 0x%x\n", x, y1) ;
	wa = fmult(x, y1, test) ;
	if (test)
		printf ("wa = 0x%x\n--------------------\n", wa) ;
	else {
		printf ("@(posedge clk)\n") ;
		printf ("begin\n") ;
		printf ("A <= 16'h%x\n", x) ;
		printf ("SR <= 11'h%x\n", y1) ;
		printf ("WA <= 16'h%x\n", wa) ;
		printf ("end\n") ;
		}
}

exit (0) ;

}

long
fmult(x, y1, test)
long	x, y1;
int	test;
{
	long	xs, xmag, xexp, xmant;
	long	ys, yexp, ymant;
	long	wxs, wx, wxexp, wxmant, wxmag, i;

	xs = (x >> 15) & 1;
	xmag = xs ? (16384 - (x >> 2)) & 8191 : x >> 2;
	xexp = 0;
	for (i = 0; i <= 13; i += 1) {
		if (!(xmag >> i)) {
			xexp = i;
			break;
		}
		if (i == 13)
			printf("mag didn't get set in floata\n");
	}
	xmant = xmag ? (xmag << 6) >> xexp : 1 << 5;

	ys = (y1 >> 10) & 1;
	yexp = (y1 >> 6) & 15;
	ymant = y1 & 63;

    if (test)
    {
    printf(" xs = %x  xexp = %x   xmant = %x\n",xs,xexp,xmant);
    printf(" ys = %x  yexp = %x   ymant = %x\n",ys,yexp,ymant);
    }

	wxs = ys ^ xs;
	wxexp = yexp + xexp;
	wxmant = ((ymant * xmant) + 48) >> 4;
	wxmag = wxexp > 26 ? ((wxmant << 7) << (wxexp - 26)) & 32767
	     : (wxmant << 7 ) >> ( 26 - wxexp);
	wx = wxs ? (65536 - wxmag) & 65535 : wxmag & 65535;
	
    if (test)
    	printf("wxs = %x wxexp = %x wxmant = %x\n",wxs,wxexp,wxmant);
	return(wx);
}
