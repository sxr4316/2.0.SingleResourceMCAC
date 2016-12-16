main()
{
	short		pcm,linear;
	short		l,p;
 
	for( pcm = 0; pcm < 16; pcm++) {
		linear = 0x1 | pcm << 1 ;
		printf("		.word	%04xh		; pcm = %x\n",linear,pcm);
	}

	for( pcm = 16; pcm < 128; pcm++) {
		l = (0x21 << (((pcm & 0xf0) >> 4) - 1) );
		p = ( (pcm & 0x0f) << ((pcm & 0xf0) >> 4) );
		linear = l | p ;
		printf("		.word	%04xh		; pcm = %x\n",linear,pcm);
	}
	
	for( pcm = 0; pcm < 16; pcm++) {
		linear = 0x1 | pcm << 1 ;
		linear = ( linear & 0xfff ) * -1;
		printf("		.word	%04xh		; pcm = %x\n",linear, pcm | 0x80);
	}

	for( pcm = 16; pcm < 128; pcm++) {
		l = (0x21 << (((pcm & 0xf0) >> 4) - 1) );
		p = ( (pcm & 0x0f) << ((pcm & 0xf0) >> 4) );
		linear = l | p ;
		linear = ( linear & 0xfff ) * -1;
		printf("		.word	%04xh		; pcm = %x\n",linear, pcm | 0x80);
	}

}
