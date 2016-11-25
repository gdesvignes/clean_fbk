#include <stdio.h>
#include <string.h>
#include <math.h>

#include "header.h"

void send_string(FILE *pfo, char const *string);
void send_float(FILE *pfo, char const *name,float floating_point);
void send_double(FILE *pfo, char const *name, double double_precision);
void send_long(FILE *pfo, char const *name, long integer);
void send_int(FILE *pfo, char const *name, int integer);
void send_coords(FILE *pfo, double raj, double dej, double az, double za);


int  write_header(FILE *pfo, header *h) {
    send_string(pfo, "HEADER_START");
    send_string(pfo, "rawdatafile");
    send_string(pfo, h->rawdatafile);
    send_string(pfo, "source_name");
    send_string(pfo, h->source_name);
    send_int(pfo, "machine_id", h->machine_id);
    send_int(pfo, "telescope_id", h->telescope_id);
    send_coords(pfo, h->src_raj, h->src_dej, 0.0,0.0);       
    send_int(pfo, "data_type", h->data_type);
    send_double(pfo, "fch1", h->fch1);
    send_double(pfo, "foff", h->foff);
    send_int(pfo, "nchans", h->nchans);
    send_int(pfo, "nbits", h->nbits);
    send_double(pfo, "tstart", h->tstart);
    send_double(pfo, "tsamp", h->tsamp);
    send_int(pfo, "nifs", h->nifs);
    send_string(pfo, "HEADER_END");
    return(1);
}

void send_string(FILE *pfo, char const *string) {
  int len;
  len=strlen(string);
  fwrite(&len, sizeof(int), 1, pfo);
  fwrite(string, sizeof(char), len, pfo);
}

void send_float(FILE *pfo, char const *name,float floating_point) {
  send_string(pfo, name);
  fwrite(&floating_point,sizeof(float),1,pfo);
}

void send_double(FILE *pfo, char const *name, double double_precision) {
  send_string(pfo, name);
  fwrite(&double_precision,sizeof(double),1,pfo);
}

void send_long(FILE *pfo, char const *name, long integer) {
  send_string(pfo, name);
  fwrite(&integer,sizeof(long),1,pfo);
}

void send_int(FILE *pfo, char const *name, int integer) {
  send_string(pfo, name);
  fwrite(&integer,sizeof(int),1,pfo);
}

void send_coords(FILE *pfo, double raj, double dej, double az, double za) {
  if ((raj != 0.0) || (raj != -1.0)) send_double(pfo, "src_raj",raj);
  if ((dej != 0.0) || (dej != -1.0)) send_double(pfo, "src_dej",dej);
  if ((az != 0.0)  || (az != -1.0))  send_double(pfo, "az_start",az);
  if ((za != 0.0)  || (za != -1.0))  send_double(pfo, "za_start",za);
}
