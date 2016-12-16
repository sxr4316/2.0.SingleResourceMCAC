long
functf(I, br)
long	I, br;
{
	long	is, fi;
	long	im;
	
	if(br==40)
	{
		is = I >> 4;
		im = is ? ((31 - I) & 15) : (I & 15);

		switch ((int)im) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			fi = 0;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			fi = 0x1;
			break;
		case 10:
			fi = 0x2;
			break;
		case 11:
			fi = 0x3;
			break;
		case 12:
			fi = 0x4;
			break;
		case 13:
			fi = 0x5;
			break;
		case 14:
		case 15:
			fi = 0x6;
			break;
		default:
			break;
		}
	}
	else if(br==32)
	{
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
			break;
		}
	}
	else if(br==24)
	{
		is = I >> 2;
		im = is ? ((7 - I) & 3) : (I & 3);

		switch ((int)im) {
		case 0:
			fi = 0;
			break;
		case 1:
			fi = 0x1;
			break;
		case 2:
			fi = 0x2;
			break;
		case 3:
			fi = 0x7;
			break;
		default:
			break;
		}
	}
	else if(br==16)
	{
		is = I >> 1;
		im = is ? ((3 - I) & 1) : (I & 1);

		switch ((int)im) {
		case 0:
			fi = 0;
			break;
		case 1:
			fi = 0x7;
			break;
		default:
			break;
		}
	}
	return(fi);
}
