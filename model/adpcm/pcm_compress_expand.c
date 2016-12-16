/*
 *
 * compress / expand PCM
 *
 */

long
inv_g711(is,imag)
long is,imag;
{
    long mag,i,s,q,ymag,inv;

    mag = imag + 33;
    if(mag >= 0x2000) ymag = 0x7f;
    else
    {
        for(i = 12; i >= 5; i--)
        {
            if(mag & (1 << i)) break;
if(i == 5)printf("mag too large inv_g711\n");
        }
        s = i - 5;
        q = (mag & ( 0x0f << (i - 4))) >> (i - 4);
        ymag = (s << 4) + q;
    }
    inv = is ? ymag | 0x80 : ymag & 0x7f;
    inv = inv ^ 0xff;
    return(inv);
}

long
g711(pcm)
long pcm;
{
    long q,s,sgn,mag;
    
    pcm ^= 0xff;
    q = 0x0f & pcm;
    s = (0x70 & pcm) >> 4;
    sgn = 0x80 & pcm;

    mag = (((2 * q) + 33) << s) - 33;
    mag = (sgn ? mag | (1 << 13) : mag) & 16383; 
    return(mag);
}
