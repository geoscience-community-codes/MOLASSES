/*############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.
#
#    Copyright (C) 2015-2021  
#    Laura Connor (lconnor@usf.edu)
#    Jacob Richardson 
#    Charles Connor
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################*/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include "include/prototypes_LJC3.h"

#define CR 13            /* Decimal code of Carriage Return char */
#define LF 10            /* Decimal code of Line Feed char */
#define MAX_LEN 256

Vent *CHOOSE_NEW_VENT(
Inputs *In, 
int ev_num,
int *num_vent) {
  
  char *token;
  int ct, ret;
  char *str;
  char prev_char;
  int vent_ct = 1;
  Vent *v = NULL;
  
	/* Now assign the vents for one event to the Vent array structure. */
	str = In->Events[ev_num];
	fprintf(stdout, "Event locations: %s\n", In->Events[ev_num]);
	ct = 0;
	
	prev_char = '\0';
	while (str[ct] != '\0') {
	  if (str[ct] == ' ' || str[ct] =='\n' || str[ct] =='\t') {
	    if (prev_char != ' ' || prev_char != '\n' || prev_char != '\t' || prev_char != '\0') vent_ct++;
	  }
	  prev_char = str[ct];
	  ct++;
	}

	v = (Vent *)GC_MALLOC( vent_ct * sizeof(Vent) );
	if (v == NULL) {
		fprintf(stderr, "Cannot malloc memory for new vent array:[%s]\n", strerror(errno));
		assert(v == NULL);
		printf("Heap size = %ld\n", GC_get_heap_size());
		return NULL;
	}
	vent_ct = 0; 	   
	token = strtok(str, " ");
		
  while ( token != NULL) {
	 //fprintf(stderr, "%s\n", token);
		  
	  while (ret = sscanf(token, "%lf,%lf", 
		             &(v+(vent_ct))->easting,
                 &(v+(vent_ct))->northing), 
                 ret != 2) {
		  if (ret == EOF && errno == EINTR) continue;
			fprintf(stderr, "vent %d: ret=%d]", vent_ct, ret);
			return NULL;
    }
		printf("\tE: %lf N: %lf\n", (v+vent_ct)->easting,  (v+vent_ct)->northing);
		vent_ct++;
  	token = strtok(NULL, " ");
  }		
  *num_vent = vent_ct;
  printf("Running molasses using %d vents\n", *num_vent );
	return v;
}
