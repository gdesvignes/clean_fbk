#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <string>

#include "header.h"
#include "sigproc_utils.h"

#include "med_filter.h"


int main(int argc, char *argv[]) {
    

    /*
    char line[64];
    float x, val, median;
    FILE *pfi;
    pfi = fopen(argv[1], "r");

    printf("Opening %s\n", argv[1]);

    while(fgets(line,64,pfi) &&  !feof(pfi)) {
        sscanf(line, "%f %f", &x, &val);
        median = medfilter<float>(val);
	printf("%f %f %f\n", x, median, val - median);
    }
    */
    
    int l, ns, bulk_size=10000;
    int s_if=-1;
    int header_size;
    char filename[128];
    long long numsamps;
    FILE *pfi, *pfo;
    unsigned char *c;
    unsigned short *s;
    float *f;

    header sh;

    while((l = getopt(argc, argv, "c:b:")) != -1) {
        switch(l) {
	    case 'c':
		sscanf(optarg,"%d",&s_if);
	        break;

	    case 'b':
		sscanf(optarg,"%d",&bulk_size);
	        break;

	    case '?':
		if (optopt == 'c')
		  fprintf (stderr, "Option -%c requires an argument.\n", optopt);
		else if (isprint (optopt))
		  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
		else
		  fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
	        return 1;
	    default:
	        abort ();
      }

    }

    strcpy(filename,argv[1]);

    // Read input file
    if((pfi=fopen(filename,"rb"))==NULL) {
        printf("Cannot open file %s\n",filename);
        exit(-2);
    }

    /* try to read the header */
    if (!(header_size=read_header(pfi,&sh))) {
        printf("Error reading header\n");
        exit(-2);
    }

    // Get the number of samples
    numsamps=nsamples(filename, header_size, sh.nbits, sh.nifs, sh.nchans);

    if (s_if>=0 && s_if < sh.nifs) {printf("Will extract IF %d\n", s_if);} 
    else {printf("IF %d not available\n", s_if); exit(-1);}

    // Malloc tmp array
    switch(sh.nbits) {
	case 8:
	    c = (unsigned char *) malloc(bulk_size * sh.nifs * sh.nchans * sizeof(char));
	    break;
	case 16:
	    s = (unsigned short *) malloc(bulk_size * sh.nifs * sh.nchans * sizeof(short));
	    break;
	case 32:
	    f = (float *) malloc(bulk_size * sh.nifs * sh.nchans * sizeof(float));
	    break;
    }
    



    // Write output file
    std::string ofilename(filename);
    ofilename = ofilename + ".clean";
    if((pfo=fopen(ofilename.c_str(),"wb"))==NULL) {
        printf("Cannot open file %s\n",ofilename.c_str());
        exit(-2);
    }
    write_header(pfo, &sh);

    int nsamp = bulk_size;
    int remain_samp = numsamps - nsamp;
    while(remain_samp) {

	switch(sh.nbits) {
	    case 8:
		fread(&c, nsamp * sh.nifs * sh.nchans * sh.nbits/8,1,pfi);
		break;
	    case 16:
		fread(&s, nsamp * sh.nifs * sh.nchans * sh.nbits/8,1,pfi);
		break;
	    case 32:
		fread(&f, nsamp * sh.nifs * sh.nchans * sh.nbits/8,1,pfi);
		break;
	    default:
		printf("Cannot read %d bits per sample...\n", sh.nbits);
		exit(-2);
	}

	remain_samp-= remain_samp;

	if (remain_samp - bulk_size > 0) {nsamp = bulk_size;} 
	else nsamp = remain_samp;

    }
	
	

    for (int i=0; i<sh.nifs; i++) {
	for (int j=0; j<sh.nchans; j++) {
	    for (int k=0; k<numsamps; k++) {
		switch(sh.nbits) {
		    case 8:
			fread(&c,sh.nbits/8,1,pfi);
			f[0]=(float) c;
			ns=1;
			break;
		    case 16:
			fread(&s,sh.nbits/8,1,pfi);
			f[0]=(float) s;
			ns=1;
			break;
		    case 32:
			fread(&f[0],sh.nbits/8,1,pfi);
			ns=1;
			break;
		    default:
			printf("Cannot read %d bits per sample...\n", sh.nbits);
			exit(-2);
		}
		

		// Next step...

		//printf("%f\n", f[0]);
	    }
	}
    }


    //printf() 

    fclose(pfi);
    fclose(pfo);

}
