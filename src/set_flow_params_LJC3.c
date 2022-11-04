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

#include "include/prototypes_LJC3.h"

int SET_FLOW_PARAMS(
Inputs *In, 
Lava_flow *active_flow, 
double *gridInfo, 
DataCell **grid)
{
	float	log_min;
	float	log_max;
	int i, j;
	
  
	if (In->min_residual > 0 && In->max_residual > 0 && 
	    In->max_residual >= In->min_residual) {	
		if (In->log_mean_residual > 0 && In->log_std_residual > 0) {
			log_min = log10(In->min_residual);
			log_max = log10(In->max_residual);
			while (active_flow->residual = 
					(double) gennor( (float) In->log_mean_residual, (float) In->log_std_residual ), 
					active_flow->residual > log_max || active_flow->residual < log_min);
			active_flow->residual = pow(10, active_flow->residual); 
		}
		else active_flow->residual = (double) genunf( (float) In->min_residual, (float) In->max_residual );
      fprintf(stdout, "Flow residual: %0.2f (meters)\n", active_flow->residual);
	}
	else active_flow->residual = In->residual;

	if (In->min_total_volume > 0 && 
	In->max_total_volume > 0 && 
	In->max_total_volume >= In->min_total_volume) 
	{
		if (In->log_mean_volume > 0 && In->log_std_volume > 0) 
		{
		  log_min = log10(In->min_total_volume);
		  log_max = log10(In->max_total_volume);
			while (active_flow->volumeToErupt = 
						(double) gennor ( (float) In->log_mean_volume, (float) In->log_std_volume ),
						active_flow->volumeToErupt > log_max || active_flow->volumeToErupt < log_min);
			active_flow->volumeToErupt = pow(10, active_flow->volumeToErupt); 
		}
		else active_flow->volumeToErupt = (double) genunf ( (float) In->min_total_volume, (float) In->max_total_volume );
		fprintf(stdout, "Total lava volume: %0.2g (cubic meters)\n", active_flow->volumeToErupt);
		active_flow->currentvolume = active_flow->volumeToErupt;
	}

	if (In->min_pulse_volume > 0 && 
		In->max_pulse_volume > 0 && 
		In->max_pulse_volume >= In->min_pulse_volume) 
	{
		active_flow->pulsevolume = 
		(double) genunf ( (float) In->min_pulse_volume, (float) In->max_pulse_volume );
		fprintf(stdout, "Flow pulse volume: %0.2g (cubic meters)\n", active_flow->pulsevolume);
	}

	/*Write residual value into 2D Global Data Array*/
	for(i=0; i < gridInfo[4]; i++) 
	{
		for(j=0; j < gridInfo[2]; j++) 
		{
			grid[i][j].residual = active_flow->residual;
		}
	}
	
	 /* Do not put vent(s) on active list 
	    Initialize vents, assign vent its grid location*/
	for (i = 0; i < active_flow->num_vents; i++) { 
	  (active_flow->source+i)->row = (int) (((active_flow->source+i)->northing - gridInfo[3]) / gridInfo[5]); /* Row (Y) of vent cell*/
	  (active_flow->source+i)->col = (int) (((active_flow->source+i)->easting - gridInfo[0]) / gridInfo[1]); /* Col (X) of vent cell*/
	  /* grid[(vent+i)->row][(vent+i)->col].active = 0; */
	}
	return(0);
}
