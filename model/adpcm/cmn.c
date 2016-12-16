/*
 *
 * common routines shared between encoder and decoder
 *
 */


/*
 *
 *	@(#) cmn.c 3.3@(#)
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include "encode.h"


extern int debug;

extern long yu;
extern long yl;
extern long dms;
extern long dml;
extern long ap;
extern long pk2;
extern long pk1;
extern long sr2;
extern long sr1;
extern long b1;
extern long b2;
extern long b3;
extern long b4;
extern long b5;
extern long b6;
extern long dq6;
extern long dq5;
extern long dq4;
extern long dq3;
extern long dq2;
extern long dq1;
extern long a2;
extern long a1;
extern long td;
extern long sez;
extern long se;
extern long sd;
extern long pcm;

long	inv_g711(), g711(), expand(), subta(), subtb();
long	quan(), adda(), antilog();
long	filtd(), functw();
long	filte(), upa2();
long	mix(), functf(), addb(), addc();
long	floata(), floatb(), fmult();
long	limc(), limb(), limd(), upa1(), calcbn();
long	upa1(), upb(), xor(), lima(), subtc();
long	filta(), filtb(), filtc();
long	compress(), synch();
long	adapt_pred(), inv_adapt_quan(), adapt_quan();
long	quan_scl_fact(), adapt_speed(), output_conv();
long	getSample();

extern struct tbl quan_tbl[];
extern long	recon_tbl[];
extern long	mult_tbl[];

long
getSample( fpin )
FILE *fpin;
{
	char	buff[3];
	long	val;

	buff[2] = (char)NULL;
	buff[0] = fgetc(fpin);
	if ( !isxdigit(buff[0]))
		buff[0] = fgetc(fpin);
	buff[1] = fgetc(fpin);
	if ( !isxdigit(buff[1]))
		buff[1] = fgetc(fpin);
	sscanf( buff, "%x", &val );

	return( val );
}


long
expand(ss, aLaw)
long	ss;
int	aLaw;
{
	long	sss, ssm, ssq, sl;

	if ( !aLaw ) {
		/* process u-law */
		sss = ss >> 13;
		ssq = ss & 8191;
	} else {
		/* process a-law */
		sss = ss >> 12;
		ssm = ss & 4095;
		ssq = ssm << 1;
	}
	sl = sss ? (16384 - ssq) & 16383 : ssq;
	return(sl);
}


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


llog(d, dl, ds)
long	d, *dl, *ds;
{
	long	i;
	long	dqm, exp, mant;

	*ds = (d >> 15) & 1;
	dqm = *ds ? (65536 - d) & 32767 : d;

	for (i = 0 ; i <= 14; i++) {
		if ((dqm >> i) <= 1) {
			exp = i;
			break;
		}
		if (i == 14)
			printf("exp too large llog\n");
	}

	mant = ((dqm << 7) >> exp) & 127;
	*dl = ((exp << 7) + mant) & 2047;
}


long
quan(dln, ds)
long	dln, ds;
{
	int	i;
	long	I;

	for (i = 0; i <= QTBL_LEN; i++) {
		I = quan_tbl[i].i;
		if ((dln >= quan_tbl[i].dln_lo)
		     && (dln <= quan_tbl[i].dln_hi)
		     && (ds == quan_tbl[i].ds)) 
			return(I);
	}
	printf("no match in quan\n");
	return(I);
}


long
subtb(dl, y1)
long	dl, y1;
{
	long	dln;

	dln = (dl + 4096 - (y1 >> 2)) & 4095;
	return(dln);
}


long
adda(dqln, y1)
long	dqln, y1;
{
	long	dql;

	dql = (dqln + (y1 >> 2)) & 4095;
	return(dql);
}


long
antilog(dql, dqs)
long	dql, dqs;
{
	long	dq, dex, dmn, dqt, dqmag, ds;

	ds = (dql >> 11) & 1;
	dex = (dql >> 7) & 15;
	dmn = dql & 127;
	dqt = (1 << 7) + dmn;
	dqmag = ds ? 0 : (dqt << 7) >> ( 14 - dex);
	dq = ((dqs << 14) + dqmag) & 32767;
	return(dq);
}


reconst(I, dqln, dqs)
long	I, *dqln, *dqs;
{
	*dqln = recon_tbl[I];
	*dqs = I >> 3;
}


long
filtd(wi, y1)
long	wi, y1;
{
	long	dif, difs, difsx, yut;

	dif = ((wi << 5) + 131072 - y1) & 131071;
	difs = dif >> 16;

	difsx = difs ? (dif >> 5) + 4096 : dif >> 5;
	yut = (y1 + difsx) & 8191;
	return(yut);
}


long
filte(yup1, yl1)
long	yup1;
long	yl1;
{
	long	dif, difs, difsx;
	long	ylp1;

	dif = ((yup1 + ((1048576 - yl1) >> 6)) & 16383);
	difs = dif >> 13;

	difsx = difs ? dif + 507904 : dif;
	ylp1 = (yl1 + difsx) & 524287;
	return(ylp1);
}


long
functw(I)
long	I;
{
	long	is, im, wi;

	is = I >> 3;
	im = is ? (15 - I) & 7 : I & 7;
	wi = mult_tbl[im];
	return(wi);
}


long
limb(yut)
long	yut;
{
	long	yup1, geul, gell;

	geul = (((yut + 11264) & 16383) >> 13) & 1;
	gell = ((yut + 15840) & 16383) >> 13;

	if (gell == 1) 
		yup1 = 544;
	else if (geul == 0) 
		yup1 = 5120;
	else 
		yup1 = yut;

	return(yup1);
}


long
mix(al1, yu1, yl1)
long	al1, yu1;
long	yl1;
{
	long	dif, difs, difm, prodm, prod, y1;

	dif = (yu1 + 16384 - (yl1 >> 6)) & 16383;
	difs = dif >> 13;

	difm = difs ? (16384 - dif) & 8191 : dif;
	prodm = (difm * al1) >> 6;
	prod = difs ? (16384 - prodm) & 16383 : prodm;
	y1 = ((yl1 >> 6) + prod) & 8191;

	return(y1);
}


long
filta(fi, dms1)
long	fi, dms1;
{
	long	dif, difs, difsx, dmsp1;

	dif = ((fi << 9) + 8192 - dms1) & 8191;
	difs = dif >> 12;

	difsx = difs ? (dif >> 5) + 3840 : dif >> 5;
	dmsp1 = (difsx + dms1) & 4095;
	return(dmsp1);
}


long
filtb(fi, dml1)
long	fi, dml1;
{
	long	dif, difs, difsx, dmlp1;

	dif = ((fi << 11) + 32768 - dml1) & 32767;
	difs = dif >> 14;

	difsx = difs ? (dif >> 7) + 16128 : dif >> 7;
	dmlp1 = (difsx + dml1) & 16383;
	return(dmlp1);
}


long
filtc(ax, ap1)
long	ax, ap1;
{
	long	dif, difs, difsx, app1;

	dif = ((ax << 9) + 2048 - ap1) & 2047;
	difs = dif >> 10;

	difsx = difs ? (dif >> 4) + 896 : dif >> 4;
	app1 = (difsx + ap1) & 1023;
	return(app1);
}


long
functf(I)
long	I;
{
	long	is, fi;
	long	im;

	is = I >> 3;
	im = is ? ((15 - I) & 7) : (I & 7);

	switch ((int)im) {
	case 0:
	case 1:
	case 2:
		fi = 0;
		break;
	case 3:
	case 4:
	case 5:
		fi = 0x1;
		break;
	case 6:
		fi = 0x3;
		break;
	case 7:
		fi = 0x7;
		break;
	default:
		printf("default found funcf\n");
	}
	return(fi);
}


long
lima(ap1)
long	ap1;
{
	long	al1;

	al1 = (ap1 >= 256) ? 64 : ap1 >> 2;
	return(al1);
}


long
subtc(dmsp1, dmlp1, y1, tdp1)
long	dmsp1, dmlp1, y1, tdp1;
{
	long	dif, difs, difm, dthr, ax;

	dif = ((dmsp1 << 2) + 32768 - dmlp1) & 32767;
	difs = dif >> 14;

	difm = difs ? (32768 - dif) & 16383 : dif;
	dthr = dmlp1 >> 3;
	ax = ((y1 >= 1536) && (difm < dthr)) && (tdp1 == 0) ? 0 : 1;
	return(ax);
}


long
triga(tr, app1)
long	tr, app1;
{
	return(tr ? 256 : app1);
}


accum(wa1, wa2, wb1, wb2, wb3, wb4, wb5, wb6)
long	wa1, wa2, wb1, wb2, wb3, wb4, wb5, wb6;
{
	long	sezi, sei;

	sezi = (((((((((wb1 + wb2) & 65535) + wb3) & 65535) + wb4) & 65535)
	     + wb5) & 65535) + wb6) & 65535;
	sei = (((sezi + wa2) & 65535) + wa1) & 65535;
	sez = (sezi >> 1) & 32767;
	se = (sei >> 1) & 32767;
}


long
addb(dq, se1)
long	dq, se1;
{
	long	dqs, dqi, ses, sei, sri;

	dqs = dq >> 14;
	dqi = dqs ? (65536 - (dq & 16383)) & 65535 : dq;
	ses = se1 >> 14;
	sei = ses ? (1 << 15) + se1 : se1;
	sri = (dqi + sei) & 65535;
	return(sri);
}


long
addc(dq, sez1, sigpk1)
long	dq, sez1, *sigpk1;
{
	long	dqs, dqi, sezs, sezi, pko, dqsez;

	dqs = (dq >> 14) & 1;
	dqi = dqs ? (65536 - (dq & 16383)) & 65535 : dq;
	sezs = sez1 >> 14;
	sezi = sezs ? (1 << 15) + sez1 : sez1;
	dqsez = (dqi + sezi) & 65535;
	pko = (dqsez >> 15) & 1;
	*sigpk1 = dqsez == 0 ? 1 : 0;
	return(pko);
}


long
floata(dq)
long	dq;
{
	long	dqs, mag, exp, mant, dqo;
	long	i;

	dqs = (dq >> 14) & 1;
	mag = dq & 16383;

	exp = 0;
	for (i = 0; i <= 14; i += 1) {
		if (!(mag >> i)) {
			exp = i;
			break;
		}
		if (i == 14)
			printf("mag didn't get set in floata\n");
	}
	mant = mag ? (mag << 6) >> exp : 1 << 5;
	dqo = ((dqs << 10) + ( exp << 6) + mant) & 2047;
	return(dqo);
}


long
floatb(sri)
long	sri;
{
	long	srs, mag, exp, mant, sro, i;

	srs = (sri >> 15) & 1;
	mag = srs ? (65536 - sri) & 32767 : sri;

	exp = 0;
	for (i = 0; i <= 15; i += 1) {
		if (!(mag >> i)) {
			exp = i;
			break;
		}
		if (i == 15)
			printf("mag didn't get set in floatb\n");
	}
	mant = mag ? (mag << 6) >> exp : 1 << 5;
	sro = ((srs << 10) + ( exp << 6) + mant) & 2047;
	return(sro);
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
			printf("mag didn't get set in fmult\n");
	}
	xmant = xmag ? (xmag << 6) >> xexp : 1 << 5;

	ys = (y1 >> 10) & 1;
	yexp = (y1 >> 6) & 15;
	ymant = y1 & 63;
/*

    if (test)
    {
    printf("bs = %x  bexp = %x   bmant = %x\n",xs,xexp,xmant);
    printf("ds = %x  dexp = %x   dmant = %x\n",ys,yexp,ymant);
    }
*/

	wxs = ys ^ xs;
	wxexp = yexp + xexp;
	wxmant = ((ymant * xmant) + 48) >> 4;
	wxmag = wxexp > 26 ? ((wxmant << 7) << (wxexp - 26)) & 32767
	     : (wxmant << 7 ) >> ( 26 - wxexp);
	wx = wxs ? (65536 - wxmag) & 65535 : wxmag & 65535;
/*
    if (test)
	printf("wb = %x\n",wx);
*/
	return(wx);
}


#define A2UL	12288
#define A2LL	53248

long
limc(a2t)
long	a2t;
{
	long	a2p1;

	if ((a2t >= 32768) && (a2t <= A2LL)) 
		a2p1 = A2LL;
	else if ((a2t >= A2UL) && (a2t <= 32767)) 
		a2p1 = A2UL;
	else 
		a2p1 = a2t;
	a2p1 &= 65535;
	return(a2p1);
}


#define OME	15360

long
limd(a1t, a2p1)
long	a1t, a2p1;
{
	long	a1ul, a1ll, a1p1;

	a1ul = (OME + 65536 - a2p1) & 65535;
	a1ll = (a2p1 + 65536 - OME) & 65535;

	if ((a1t >= 32768) && (a1t <= a1ll)) 
		a1p1 = a1ll;
	else if ((a1t >= a1ul) && (a1t <= 32767)) 
		a1p1 = a1ul;
	else 
		a1p1 = a1t;
	return(a1p1);
}


long
trigb(tr, anp)
long	tr, anp;
{
	return(tr ? 0 : anp);
}


long
upa1(pk01, pk11, a11, sigpk1)
long	pk01, pk11, a11, sigpk1;
{
	long	pks, uga1, a1s, ula1, ua1, a1t;

	pks = pk01 ^ pk11;
	if ((pks == 0) && (sigpk1 == 0))
		uga1 = 192;
	else if ((pks == 1) && (sigpk1 == 0))
		uga1 = 65344;
	else
		uga1 = 0;
	a1s = (a11 >> 15) & 1;
	ula1 = a1s ? (65536 - (( a11 >> 8) + 65280)) & 65535
	     : (65536 - (a11 >> 8)) & 65535;
	ua1 = (uga1 + ula1) & 65535;
	a1t = ( a11 + ua1) & 65535;
	return(a1t);
}


long
upa2(pk01, pk11, pk21, a11, a21, sigpk1)
long	pk01, pk11, pk21, a11, a21, sigpk1;
{
	long	pks1, pks2, uga2a, a1s, fa1, fa, uga2s, uga2b, a2s, ula2, uga2, ua2, a2t;

	pks1 = pk01 ^ pk11;
	pks2 = pk01 ^ pk21;
	/* 114688 */
	uga2a = pks2 ? 114688 : 16384;
	a1s = (a11 >> 15) & 1;
	if (!a1s) 
		fa1 = (a11 >= 8192) ? 8191 << 2 : a11 << 2;
	else 
		fa1 = (a11 <= 57344) ? 24577 << 2 : (a11 << 2) & 131071;
	fa = pks1 ? fa1  : (131072 - fa1) & 131071;
	uga2b = (uga2a + fa) & 131071;
	uga2s = (uga2b >> 16) & 1;
	if ((uga2s == 0) && (sigpk1 == 0))
		uga2 = uga2b >> 7;
	else if ((uga2s == 1) && (sigpk1 == 0))
		uga2 = (uga2b >> 7) + 64512;
	else
		uga2 = 0;
	a2s = (a21 >> 15) & 1;
	ula2 = a2s ? (65536 - ((a21 >> 7) + 65024)) & 65535
	     : (65536 - (a21 >> 7)) & 65535;
	ua2 = (uga2 + ula2) & 65535;
	a2t = (a21 + ua2) & 65535;
	return(a2t);
}


long
upb(un, bn, dq)
long	un, bn, dq;
{
	long	ugbn, bns, ulbn, ubn, bnp, dqmag;

	dqmag = dq & 16383;
	if ((un == 0) && (dqmag != 0))
		ugbn = 128;
	else if (un && (dqmag != 0))
		ugbn = 65408 ;
	else
		ugbn = 0;
	bns = bn >> 15;
	ulbn = bns ? (65536 - ((bn >> 8) + 65280)) & 65535 
	     : (65536 - (bn >> 8)) & 65535;
	ubn = (ugbn + ulbn) & 65535;
	bnp = (bn + ubn) & 65535;
	return(bnp);
}


long
tone(a2p)
long	a2p;
{
	long det = 0 ;

	if ((32768 <= a2p) && (a2p < 53760)) 
		det = 1;

	if (debug) {
		printf("tone a2p:  %06x(%6d)\n", a2p, (short) a2p);
		printf("tone det:  %06x\n", det);
	}

	return(det);
}


long
trans(td1, yl1, dq)
long	td1, yl1, dq;
{
	long	tr, dqmag, ylint, ylfrac, thr1, thr2, dqthr;

	dqmag = dq & 16383;
	ylint = (yl1 >> 15) & 0x0f;
	ylfrac = (yl1 >> 10) & 31;
	thr1 = (32 + ylfrac) << ylint;
	thr2 = ylint > 8 ? 31 << 9 : thr1;
	dqthr = (thr2 + (thr2 >> 1)) >> 1;
	tr = (dqmag > dqthr) && (td1 == 1) ? 1 : 0;

	if (debug) {
		printf("trans td1:    %06x\n", td1);
		printf("trans yl1:    %06x\n", yl1);
		printf("trans dq:     %06x\n", dq);
		printf("trans dqmag:  %06x\n", dqmag);
		printf("trans ylint:  %06x\n", ylint);
		printf("trans ylfrac: %06x\n", ylfrac);
		printf("trans thr1:   %06x\n", thr1);
		printf("trans thr2:   %06x\n", thr2);
		printf("trans dqthr:  %06x\n", dqthr);
		printf("trans tr:     %06x\n\n", tr);
	}
	return(tr);
}


long
xor(dqn, dq)
long	dqn, dq;
{
	long	dqs, dqns, un;

	dqs = (dq >> 14) & 1;
	dqns = (dqn >> 10) & 1;
	un = (dqs ^ dqns) & 1;
	return(un);
}


long
compress(sri, aLaw)
long	sri;
int	aLaw;
{
	long	is, im, imag, sp;

	is = (sri >> 15) & 1;
	im = is ? (65536 - sri) & 32767 : sri;

	if ( !aLaw ) {
		/* process u-pcm */
		imag = im;
	} else {
		/* process a-pcm */
		im=(sri > 65535) ? 2 : im;

        /* Next line added by J.Patel to fix a with test vector ri40fa.o */
        /* for 40kbps only */
        im = ( sri == 0x8000) ? 2 : im;

		imag = is ? ((im + 1) >> 1) : (im >> 1);
		if(is)
			--imag;
		if (imag > 4095)
			imag = 4095;
	}

	sp = inv_g711(is, imag, aLaw);

	if (debug) {
		printf("compress aLaw: %06x\n", aLaw);
		printf("compress is:   %06x\n", is);
		printf("compress imag: %06x\n", imag);
		printf("compress sp:   %06x\n\n", sp);
	}

	sp = sp & 0xFF;
	return(sp);
}


long
synch(I, sp, dlnx, dsx, aLaw)
long	I, sp, dlnx, dsx;
int aLaw;
{
	long	i, is, im, id, sd;

	is = I >> 3;
	im = is ? I & 7 : I + 8;


	for (i = 0; i <= QTBL_LEN ; i++) {
		if ((dsx == quan_tbl[i].ds) && (dlnx >= quan_tbl[i].dln_lo)
		     && (dlnx <= quan_tbl[i].dln_hi)) {
			id = quan_tbl[i].id;
			break;
		}
		if (i == QTBL_LEN)
			printf("id not set in synch\n");
	}

	if (debug) {
		printf("synch aLaw: %06x\n", aLaw);
		printf("synch I:    %06x\n", I);
		printf("synch is:   %06x\n", is);
		printf("synch im:   %06d\n", im);
		printf("synch sp:   %06x\n", sp);
		printf("synch dlnx: %06d\n", dlnx);
		printf("synch dsx:  %06x\n", dsx);
		printf("synch id:   %06d\n", id);
	}

	if ( !aLaw ) {
		/* u-law pcm */
		if (id == im) 
			sd = sp;
		else
		{
			switch ((int)sp) {
			case 0x80:
				if (id > im) 
					sd = sp + 1;
				else 
					sd = sp;
				break;
			case 0xff:
				if (id > im) 
					sd = 0x7e;
				else 
					sd = sp - 1;
				break;
			case 0x00:
				if (id > im) 
					sd = sp;
				else 
					sd = sp + 1;
				break;
			case 0x7f:
				if (id > im) 
					sd = sp - 1;
				else 
					sd = 0xfe;
				break;
			default:
				if (sp & 0x80) {
					if (id > im) 
						sd = sp + 1;
					else 
						sd = sp - 1;
				} else
				 {
					if (id > im) 
						sd = sp - 1;
					else 
						sd = sp + 1;
				}
				break;
			}
		}
	} else {
		/* a-law pcm */
		sp ^= 0x55 ;
		if (id == im) {
			sd = sp;
		} else {
			switch (sp) {
			case 0xff:	/* + max */
				if (id > im) 
					sd = 0xFE;
				else 
					sd = sp;
				break;
			case 0x80:	/* + 0 */
				if (id > im) 
					sd = 0x00;
				else 
				//	sd = sp - 1;
					sd = 0x81;
				break;
			case 0x00:	/* - 0 */
				if (id > im)
					sd = 0x01;
				else
					sd = 0x80;
				break;
			case 0x7F:	/* - max */
				if (id > im) 
					sd = sp;
				else 
					sd =0x7E ;
				break;
			default:
				if (sp & 0x80) {
					if (id > im) 
						sd = sp - 1;
					else 
						sd = sp + 1;
				} else {
					if (id > im) 
						sd = sp + 1;
					else 
						sd = sp - 1;
				}
				break;
			}
		}
		sd ^= 0x55 ;
	}

	sd = sd & 0xFF;
	return(sd);
}


/* encode sign and mangnitude to pcm */
long
inv_g711(is, imag, aLaw)
long	is, imag;
int	aLaw;
{
	long	mag, i, s, q, inv;
	long	ymag = 0;
	short	iesp;

	if ( !aLaw ) {
		/* process u-law pcm */
/*
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *	Biased Linear Input Code	Compressed Code
 *	------------------------	---------------
 *	00000001wxyza			    000wxyz
 *	0000001wxyzab			    001wxyz
 *	000001wxyzabc			    010wxyz
 *	00001wxyzabcd			    011wxyz
 *	0001wxyzabcde			    100wxyz
 *	001wxyzabcdef			    101wxyz
 *	01wxyzabcdefg			    110wxyz
 *	1wxyzabcdefgh			    111wxyz
 *
 * Each biased linear code has a leading 1 which identifies the segment
 * number. The value of the segment number is equal to 7 minus the number
 * of leading 0's. The quantization interval is directly available as the
 * four bits wxyz.  * The trailing bits (a - h) are ignored.
 *
 * Ordinarily the complement of the resulting code word is used for
 * transmission, and so the code word is complemented before it is returned.
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 *
 */
		mag = imag + 33;
		if (mag >= 0x2000) {
			ymag = 0x7f;
		} else {
			for (i = 12; i >= 5; i--) {
				if (mag & (1 << i))
					break;
				if (i == 5)
					printf("mag too large inv_g711\n");
			}
			s = i - 5;
			q = (mag & ( 0x0f << (i - 4))) >> (i - 4);
			ymag = (s << 4) + q;
		}
		inv = is ? ymag | 0x80 : ymag & 0x7f;
		if (debug) {
			printf("inv_g711 aLaw: %06x\n", aLaw);
			printf("inv_g711 is:   %06x\n", is);
			printf("inv_g711 mag:  %06x\n", mag);
			printf("inv_g711 i:    %06x\n", i);
			printf("inv_g711 s:    %06x\n", s);
			printf("inv_g711 q:    %06x\n", q);
			printf("inv_g711 ymag: %06x\n", ymag);
			printf("inv_g711 inv:  %06x\n", inv);
		}
		/* invert bits */
		inv = inv ^ 0xff;
		if (debug) {
			printf("inv_g711 inv:  %06x\n\n", inv);
		}

	} else {
		/* process a-law pcm */
/*
 *
 *	Linear Input Code	        Compressed Code
 *	------------------------	---------------
 *	0000000wxyza			    000wxyz
 *	0000001wxyza			    001wxyz
 *	000001wxyzab			    010wxyz
 *	00001wxyzabc			    011wxyz
 *	0001wxyzabcd			    100wxyz
 *	001wxyzabcde			    101wxyz
 *	01wxyzabcdef			    110wxyz
 *	1wxyzabcdefg			    111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 *
 */

		ymag = imag;
	    iesp = 7;
	    for (i = 1; i <= 7; ++i)
	    {
			ymag += ymag;
			if (ymag >= 4096)
				break;
			iesp = 7 - i;
	    }
	
	    ymag &= 4095;
	
	    ymag = (ymag >> 8);
	    inv = (is == 0) ? ymag + (iesp << 4) : ymag + (iesp << 4) + 128;
	    //
	    // Sign bit inversion //
	    inv ^= 0x55;
	    inv ^= 0x80;
	
		if (debug) {
			printf("inv_g711 aLaw: %06x\n", aLaw);
			printf("inv_g711 is:   %06x\n", is);
			printf("inv_g711 iesp:  %06x\n", mag);
			printf("inv_g711 i:    %06x\n", i);
			printf("inv_g711 ymag: %06x\n", ymag);
			printf("inv_g711 inv:  %06x\n", inv);
		}
	}
	return(inv);
}


/* decode pcm to sign and mangnitude */
long
g711(pcm, aLaw)
long	pcm;
int	aLaw;
{
	long	q, s, sgn, mag, bias;

	if ( !aLaw ) {
		/* process u-law pcm */
		if (debug) {
			printf("g711 aLaw: %06x\n", aLaw);
			printf("g711 pcm:  %06x\n", pcm);
		}
		pcm ^= 0xff;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;
		bias = 33;

		mag = (((2 * q) + bias) << s) - bias;

		if (debug) {
			printf("g711 pcm:  %06x\n", pcm);
			printf("g711 q:    %06x\n", q);
			printf("g711 s:    %06x\n", s);
			printf("g711 sgn:  %06x\n", sgn);
			printf("g711 bias: %06x\n", bias);
			printf("g711 mag:  %06x(%6d)\n", mag, sgn ? ~mag + 1 : mag);
		}

		mag = (sgn ? mag | (1 << 13) : mag) & 16383;

		if (debug) {
			printf("g711 mag:  %06x\n\n", mag);
		}
	} else {
		/* process a-law pcm */
		if (debug) {
			printf("g711 aLaw: %06x\n", aLaw);
			printf("g711 pcm:  %06x\n", pcm);
		}
		pcm ^= 0x55;
		q = 0x0f & pcm;
		s = (0x70 & pcm) >> 4;
		sgn = 0x80 & pcm;

		bias = (s > 0) ? 0x00021 : 0x00001;

		mag = (2 * q) + bias;
		mag = (s > 1) ? mag << (s - 1) : mag;

		if (debug) {
			printf("g711 pcm:  %06x\n", pcm);
			printf("g711 q:    %06x\n", q);
			printf("g711 s:    %06x\n", s);
			printf("g711 sgn:  %06x\n", sgn);
			printf("g711 bias: %06x\n", bias);
			printf("g711 mag:  %06x(%6d)\n", mag, sgn ? mag : ~mag + 1);
		}

		mag = (sgn ? mag : mag | (1 << 12)) & 8191;

		if (debug) {
			printf("g711 mag:  %06x\n\n", mag);
		}
	}
	return(mag);
}

void
dumpState(i, aLaw, mode)
int i;
int	aLaw;
int	mode;
{

printf("\n=====================\n\n");
printf("cnt:   %6d\n", i);
if (mode)
	printf("mode:  %6s\n", "ENC");
else
	printf("mode:  %6s\n", "DEC");
if (aLaw)
	printf("Law:   %6s\n", "a-law");
else
	printf("Law:   %6s\n", "u-law");
printf("sez:   %06x\n", sez);
printf("se:    %06x\n", se);
printf("yu:    %06x\n", yu);
printf("yl:    %06x\n", yl);
printf("dms:   %06x\n", dms);
printf("dml:   %06x\n", dml);
printf("ap:    %06x\n", ap);
printf("pk2:   %06x\n", pk2);
printf("pk1:   %06x\n", pk1);
printf("sr2:   %06x\n", sr2);
printf("sr1:   %06x\n", sr1);
printf("b1:    %06x\n", b1);
printf("b2:    %06x\n", b2);
printf("b3:    %06x\n", b3);
printf("b4:    %06x\n", b4);
printf("b5:    %06x\n", b5);
printf("b6:    %06x\n", b6);
printf("dg6:   %06x\n", dq6);
printf("dg5:   %06x\n", dq5);
printf("dg4:   %06x\n", dq4);
printf("dg3:   %06x\n", dq3);
printf("dg2:   %06x\n", dq2);
printf("dg1:   %06x\n", dq1);
printf("a2:    %06x\n", a2);
printf("a1:    %06x\n", a1);
printf("td:    %06x\n", td);
printf("sd:    %06x\n", sd);
printf("pcm:   %06x\n", pcm);
printf("\n=====================\n\f\n");

}

void dumpPCM(long pcm, long i, long aLaw)
{
	long ss, is, imag, npcm, tc;

	ss = g711(pcm, aLaw);   /* expand 8 bits to 14/13 bit */
	is = aLaw ? ss >> 12 : ss >> 13 ;
	imag = aLaw ? (ss & 4095) : (ss & 8191) ;
	npcm = inv_g711(is, imag, aLaw);    /* compress 14/13 bit to 8 bits */
	tc = is ? (~imag + 1) : imag ;
	if (aLaw) {
		fprintf( stderr, "%8d i:%02x(^%02x)  do:%04x(%5d)  ro:%02x(^%02x)\n",
		i, pcm, pcm ^ 0x55, ss, tc, npcm, npcm ^ 0x55 );
	} else {
		fprintf( stderr, "%8d i:%02x  do:%04x(%5d)  ro:%02x\n", i, pcm, ss, tc, npcm );
	}
}
