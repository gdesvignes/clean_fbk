#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <string>

#ifdef HAVE_GSL
#include <gsl/gsl_statistics.h>
#endif

#include "header.h"
#include "sigproc_utils.h"

#include "ekstrom.h"


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
    
    int threshold = 6;
    int numfiles = 0;
    int l, ns, boxsize=10000;
    int s_if=-1;
    int header_size;
    int oldper=0, newper=0;
    char filename[64][128];
    long long numsamps;
    FILE *pfi, *pfo;
    float f[8], val, median, mean, std; // Temp data
    unsigned char c;
    unsigned short s;
    unsigned int tmpbuf_idx=0;
    int do_zap_chan = 0;

    header iheader, oheader;

    while((l = getopt(argc, argv, "c:b:t:z")) != -1) {
        switch(l) {
	    case 'c':
		sscanf(optarg,"%d",&s_if);
	        break;
	    case 'b':
		sscanf(optarg,"%d",&boxsize);
	        break;
	    case 't':
		sscanf(optarg,"%d",&threshold);
	        break;
	    case 'z':
		do_zap_chan=1;
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

    for (int i=optind; i<argc; i++) {strcpy(filename[numfiles],argv[i]); numfiles++;}

    // Read first input file
    if((pfi=fopen(filename[0],"rb"))==NULL) {
        printf("Cannot open file %s\n",filename[0]);
        exit(-2);
    } else {
	printf("Opening file %s\n",filename[0]);
    }

    /* try to read the header */
    if (!(header_size=read_header(pfi,&iheader))) {
        printf("Error reading header\n");
        exit(-2);
    }

    // Copy header infos
    memcpy(&oheader, &iheader, sizeof(header));

    // Check if specific IF has been selected
    if (s_if>=0 && s_if < iheader.nifs) {printf("Will extract IF %d\n", s_if); oheader.nifs=1;} 
    //else {printf("IF %d not available\n", s_if); exit(-1);}

    // Output is 32 bits
    oheader.nbits=32;

    // Write output file
    std::string ofilename(filename[0]);
    ofilename = ofilename + ".clean";
    if((pfo=fopen(ofilename.c_str(),"wb"))==NULL) {
        printf("Cannot open file %s\n",ofilename.c_str());
        exit(-2);
    }
    write_header(pfo, &oheader);

    // Init struct for running median
    RunningMean<float> *rm; 
    rm = new RunningMean<float>[iheader.nifs * iheader.nchans];

    for (int i=0; i < iheader.nifs * iheader.nchans; i++)
	rm[i].set_boxsize(boxsize);
    //	stats[i].buffer = (pair *) malloc(bulk_size * sizeof (pair));


    float *tmpbuf, *tmpbuf_ptr;
    tmpbuf = new float[boxsize * iheader.nifs * iheader.nchans];
    tmpbuf_ptr = tmpbuf;

    for (int h=0; h<numfiles; h++) {

	// Close current file and open new one
	if (h>1) {
	    fclose(pfi);

	    // Read first input file
	    if((pfi=fopen(filename[h],"rb"))==NULL) {
		printf("Cannot open file %s\n",filename[h]);
		exit(-2);
	    } else {
		printf("Opening file %s\n",filename[h]);
	    }

	    /* try to read the header */
	    if (!(header_size=read_header(pfi,&iheader))) {
		printf("Error reading header\n");
		exit(-2);
	    }
		    
	}

	oldper = 0;
	printf("Amount Complete = %3d%%", oldper);
	fflush(stdout);


	// Get the number of samples
	numsamps=nsamples(filename[h], header_size, iheader.nbits, iheader.nifs, iheader.nchans);

	for (int i=0; i<numsamps; i++) {
	    for (int j=0; j<iheader.nifs; j++) {
		for (int k=0; k<iheader.nchans; k++) {
		    switch(iheader.nbits) {
			case 8:
			    fread(&c,iheader.nbits/8,1,pfi);
			    f[0]=(float) c;
			    ns=1;
			    break;
			case 16:
			    fread(&s,iheader.nbits/8,1,pfi);
			    f[0]=(float) s;
			    ns=1;
			    break;
			case 32:
			    fread(&f[0],iheader.nbits/8,1,pfi);
			    ns=1;
			    break;
			default:
			    printf("Cannot read %d bits per sample...\n", iheader.nbits);
			    exit(-2);
		    }

		    if (do_zap_chan && k==0) val = 0.0;
		    else val = f[0];
		    /*else{
			// Remove median for specific IF, chan
			median = rm[j*iheader.nchans+k].get_value(f[0]);
			val = f[0] - median;
			//fwrite(&val, sizeof(float), 1, pfo);
		    }*/
			
		    // Copy to mem buffer
		    *tmpbuf_ptr = val; tmpbuf_ptr++;
		}
	    }
	    tmpbuf_idx++;
		    
	    // If buffer is full
	    if (tmpbuf_idx == boxsize) {
#ifdef HAVE_GSL
		// make stats and replace outlier
		for(int j=0; j<iheader.nchans * iheader.nifs; j++) {
		    mean = gsl_stats_float_mean(&tmpbuf[j], iheader.nchans * iheader.nifs, boxsize);
		    median = rm[j].get_value(mean);
		    std = gsl_stats_float_sd_m(&tmpbuf[j], iheader.nchans * iheader.nifs, boxsize, median);

		    for(int k=0; k<boxsize; k++) {
			tmpbuf[k*iheader.nchans * iheader.nifs + j] -= median;
			if (tmpbuf[k*iheader.nchans * iheader.nifs + j] < -threshold * std) tmpbuf[k*iheader.nchans * iheader.nifs + j] = 0.;
		    }

		}
#endif
		
		// Reset
		tmpbuf_ptr = tmpbuf;
		tmpbuf_idx=0;

		// write buffer
		fwrite(tmpbuf, sizeof(float), boxsize * iheader.nchans * iheader.nifs, pfo);
	    }


	    newper = (int) ((float)i/(float)numsamps * 100.);
	    if (newper > oldper) {
		printf("\rAmount Complete = %3d%%", newper);
		fflush(stdout);
		oldper = newper;
	    }
	}
	
#ifdef HAVE_GSL
	// make stats and replace outlier
	for(int j=0; j<iheader.nchans * iheader.nifs; j++) {
	    mean = gsl_stats_float_mean(&tmpbuf[j], iheader.nchans * iheader.nifs, tmpbuf_idx);
	    median = rm[j].get_value(mean);
	    std = gsl_stats_float_sd_m(&tmpbuf[j], iheader.nchans * iheader.nifs, tmpbuf_idx, mean);
	    for(int k=0; k<tmpbuf_idx; k++) {
		tmpbuf[k*iheader.nchans * iheader.nifs + j] -= median;
		if (tmpbuf[k*iheader.nchans * iheader.nifs + j] < - threshold * std) tmpbuf[k*iheader.nchans * iheader.nifs + j] = 0.;
	    }

	}
#endif
		
		
	// write buffer
	fwrite(tmpbuf, sizeof(float), tmpbuf_idx * iheader.nchans * iheader.nifs, pfo);


	printf("\rAmount Complete = 100%%\n");
    }


    //printf() 

    fclose(pfi);
    fclose(pfo);

}
