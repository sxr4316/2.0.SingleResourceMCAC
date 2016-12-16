long
fmult(x, y1, test)
long	x, y1;
int	test;
{
	long	xs, xmag, xexp, xmant;
	long	ys, yexp, ymant;
	long	wxs, wx, wxexp, wxmant, wxmag, i;

	xs = (x >> 15);
	xmag = xs ? (16384 - (x >> 2)) & 8191 : x >> 2;
	xexp = 0;
	for (i = 0; i <= 13; i += 1) {
		if (!(xmag >> i)) {
			xexp = i;
			break;
		}
	}
	
	xmant = xmag ? (xmag << 6) >> xexp : 1 << 5;

	ys = (y1 >> 10) ;
	yexp = (y1 >> 6) & 15;
	ymant = y1 & 63;

	wxs = ys ^ xs;
	wxexp = yexp + xexp;
	wxmant = ((ymant * xmant) + 48) >> 4;
	wxmag = wxexp > 26 ? ((wxmant << 7) << (wxexp - 26)) & 32767
	     : (wxmant << 7 ) >> ( 26 - wxexp);
	wx = wxs ? (65536 - wxmag) & 65535 : wxmag ;

	return(wx);
}

