#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "include/prototypes_LJC.h"

#define CR 13            /* Decimal code of Carriage Return char */
#define LF 10            /* Decimal code of Line Feed char */

/* PERL version: 
sub choose_new_vent {
    
  my $spatial_density = $_[0];
  my $num_vents = $_[1];
  my $sd_spacing = $_[2];
  my @lambda;
  my $sum;
  my $random;
  my $sum_lambda = 0;
  
  open(SP_DENSITY, "+<$spatial_density") or die ("cannot open $spatial_density: $!");
	my  @lines = <SP_DENSITY>;
  close SP_DENSITY;
  
  # Sum data values over area of interest
  for my $i (0..$#lines) {
    (my $east, my $north, my $data) = split(" ", $lines[$i]);
    $sum_lambda += $data;
    $lambda[$i]{east} = $east;
    $lambda[$i]{north} = $north;
    $lambda[$i]{data} = $data;
  }
  
  printf STDERR "spatial density sums to: %g\n", $sum_lambda;
  my $half = $sd_spacing/2;
  my $new_east; my $new_north;
  for (my $vent = 0; $vent < $num_vents; $vent++) {
    # Choose a random number between 0 and calculated sum of data values
    $random = random_uniform(1,0,$sum_lambda);
  
    $sum = 0;
    my $i = 0;
    #printf "Random Value chosen: %g\n",$random;
    # Select grid for new vent based on the random value
    while ($sum < $random) {
      $sum += $lambda[$i]{data};
      $i++;
    }
    # Choose random and northing and easting for new vent within chosen grid cell
    my $left = $lambda[$i]{east} - $half;
    my $right = $lambda[$i]{east} + $half;
    $new_east = random_uniform_integer(1,$left,$right);
    
    my $top = $lambda[$i]{north} + $half;
    my $bot = $lambda[$i]{north} - $half;
    $new_north = random_uniform_integer(1,$bot,$top);
    #printf STDERR "%.1f  %.1f\n",  $new_east, $new_north;
  }
  return $new_east, $new_north;
}

*/

int CHOOSE_NEW_VENT(
Inputs *In, 
VentArr *vent) 
{
	double sum_lambda = 0, sum = 0, half, random, left, right, top, bot, new_east, new_north;
	int ct, i, num_grids, grid_spacing;
	SpatialDensity *grid = vent->spd_grd;
	
	num_grids = In->num_grids;
	grid_spacing = In->spd_grid_spacing;
	
	/*Sum data values over area of interest*/
	for (ct = 0; ct < num_grids; ct++) 
	{
		sum_lambda += (grid+ct)->prob;
	}
#ifdef PRINT 
	fprintf (stderr, "spatial density sums to: %0.2g\n", sum_lambda);
#endif
	half = (double)grid_spacing/2.0;
	/* Choose a random number between 0 and calculated sum of data values */
	random = (double) genunf ( (float) 0, (float) sum_lambda ); /*random_uniform(1,0,$sum_lambda); */
	sum = 0;
	i = 0;
#ifdef PRINT 
	fprintf (stderr, "Random Value chosen: %g\n",random); */
#endif
	/* Select grid value for new vent based on the random value */
	while (sum < random) 
	{
		sum += (grid + i++)->prob;
	}
	 /* Choose random and northing and easting for new vent within chosen grid cell */
	left = (double)(grid + i)->easting - half;
	right = (double)(grid + i)->easting + half;
	new_east = (double) ignuin ( (int) left, (int) right ); /*random_uniform_integer(1,$left,$right); */
	top = (grid + i)->northing + half;
	bot = (grid + i)->northing - half;
	new_north = (double) ignuin ( (int) bot, (int) top); /* random_uniform_integer(1,$bot,$top) */
#ifdef PRINT 
	fprintf (stderr, "%f  %f ",  new_east, new_north);
#endif
	vent->easting = new_east;
	vent->northing = new_north;
#ifdef PRINT 
	fprintf (stderr, " New Vent [%0.0f  %0.0f]\n",  vent->easting, vent->northing);
#endif
	return 0;
}

/************************************
returns number of rows in file
************************************/
int count_rows(
char file[], 
long len) 
{
	int NumRows = 0;
	long totc = 0L;
	char * fp;

	fp = file;
	while (totc < len) 
	{ 
		while (*fp != LF && *fp != CR) 
		{   
			fp++;
			totc++;
		}    
		while (*fp == CR || *fp == LF) 
		{  
			totc++; 
			fp++;
		}
		NumRows++; 
	}
	return NumRows;    
}

/**************************************
reads spatial density values from file
and loads array structure (grid)
**************************************/
int load_spd_data(
FILE *Opener, 
VentArr *vent, 
int *ct) 
{
	long len, __attribute__((__unused__)) num;
	char *lines, *fp;
	int Nrows;
	long totc = 0L;
	long ind = 0L;
	char *here, *one_line;
	
	fseek(Opener, 0L, SEEK_END);  /* Position to end of file */
	len = ftell(Opener);          /* Get file length */
	rewind(Opener);               /* Back to start of file */
	lines = (char *)GC_MALLOC((size_t)((len + 1) * sizeof(char)));
	if (lines == NULL ) 
	{
		fprintf(stderr, 
					"\n[load_spd_data]: Insufficient memory to read spatial density filefile: %s (%u)\n", 
					strerror(errno), errno);
		return 1;
	}
	num = fread(lines, len, 1, Opener); /* Read the entire file into array */
	lines[len] = '\0';
	Nrows = count_rows(lines, len);
#ifdef PRINT 
	fprintf(stderr, "\nReading %ld file with %ld bytes and %d rows ", num, len, Nrows);
#endif
	vent->spd_grd = (SpatialDensity *)GC_MALLOC(( (size_t)Nrows * sizeof(SpatialDensity)));
	if (vent->spd_grd == NULL) 
	{
		fprintf(stderr, 
					"\n[load_spd_data]: Cannot malloc memory for spatial densty grid:[%s] (%u)\n", 
					strerror(errno), errno);
	return 1;
	}
	fp = lines;
	*ct = 0;
	while (totc < len) 
	{
		ind = 0L;          
		here = fp; 
		while (*fp != LF && *fp != CR) 
		{   
			fp++;
			totc++;
			ind++;
		}
		while (*fp == CR || *fp == LF) 
		{
			totc++; 
			fp++;
		}
		one_line = (char *)GC_MALLOC((ind+1) * sizeof(char));
		if (one_line == NULL) 
		{
			fprintf(stderr, 
						"\n[load_spd_data]: Cannot malloc memory for line:%s (%u)\n", 
						strerror(errno), errno);
			return 1;
		}
		strncpy(one_line, here, ind);
		one_line[ind] = '\0';
		if (one_line[0] == '#' || one_line[0] == LF || one_line[0] == ' '|| one_line[0] == CR) continue;
		/*print incoming parameter*/
		/*split line into 3 number separated by space*/
		sscanf (one_line,
					"%lf %lf %Lf\n", 
					&(vent->spd_grd + *ct)->easting, 
					&(vent->spd_grd + *ct)->northing, 
					&(vent->spd_grd + *ct)->prob); 
		(*ct)++;
	}
	return 0;
}
