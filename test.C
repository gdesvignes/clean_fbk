#include <stdio.h>

#include "med_filter.h"


int main(int argc, char *argv[]) {
    
    //int tmp[18] = {0,1,2,3,4,5,6,7,8,9,8,7,6,5,4,3,2,1};
    //for (int i=0; i<10; i++) printf("%d\n", medfilter(tmp[i]));

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

    

}
