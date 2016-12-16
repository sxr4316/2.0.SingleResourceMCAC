main()
{
	short		pcm,linear;
	short		l,p;
 
	for( pcm = 0; pcm < 128; pcm++) {
		l = (0x21 << ((pcm & 0xf0) >> 4) );
		p = ( (pcm & 0x0f) << (((pcm & 0xf0) >> 4) + 1) );
		linear = ( l | p ) - 0x21 ;
		printf("		.word	%04xh		; pcm = %x\n",linear,pcm);
	}

	for( pcm = 0; pcm < 128; pcm++) {
		l = (0x21 << ((pcm & 0xf0) >> 4) );
		p = ( (pcm & 0x0f) << (((pcm & 0xf0) >> 4) + 1) );
		linear = ( 0x2000 | l | p ) - 0x21 ;
		linear = ( linear & 0x1fff ) * -1;
		printf("		.word	%xh		; pcm = %x\n",linear, pcm | 0x80);
	}

}	
