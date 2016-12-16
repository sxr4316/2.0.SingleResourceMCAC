
main()
{

    long bs,bexp,bmant;
    long ds,dexp,dmant;
    long wxs,wx,wxexp,wxmant,wxmag,i;
    long zxs,zx,zxexp,zxmant,zxmag;

printf("dexp	bexp	dmant	bmant	wx \n");
printf("===================================\n");
for( dexp = 0; dexp <= 15; dexp++) {
 for( bexp = 0; bexp <= 13; bexp++) {
  for( dmant = 0; dmant <= 63; dmant++) {
   for( dmant = 0; dmant <= 63; dmant++) {
    	wxs = 0; 
    	wxexp = dexp + bexp;
    	wxmant = ((dmant * bmant) + 48) >> 4;
    	wxmag = wxexp > 26 ? ((wxmant << 7) << (wxexp - 26)) & 32767
	: (wxmant << 7 ) >> ( 26 - wxexp);
    	wx = wxs ? (65536 - wxmag) & 65535 : wxmag & 65535;
	zxmant = bmant * (dmant*8) + 384;
	zxmag = zxmant & 0xff80;
	i = wx<<15;
	
	if(wx!=0)
	printf("%x	%x	%x	%x	%x\n",dexp,bexp,dmant,bmant,wx);
	}
      }
    }
   }

}
