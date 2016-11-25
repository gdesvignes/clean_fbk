/*  ======================================================================  */
/*                                                                          */
/*  This is to find the size of the file by name "name". The out put is     */
/*  in terms of bytes.       R. Ramachandran, 30-juli-97, nfra.             */
/*                                                                          */
/*  ======================================================================  */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>


long long sizeof_file(char name[]) /* includefile */
{
     struct stat stbuf;

     if(stat(name,&stbuf) == -1)
     {
          fprintf(stderr, "f_siz: can't access %s\n",name);
          exit(0);
     }
     return(stbuf.st_size);
}

long long nsamples(char *filename,int headersize, int nbits, int nifs, int nchans) /*includefile*/
{
  long long datasize,numsamps;
  datasize=sizeof_file(filename)-headersize;
  numsamps=(long long) (long double) (datasize)/ (((long double) nbits) / 8.0)
                 /(long double) nifs/(long double) nchans;
  return(numsamps);
}


