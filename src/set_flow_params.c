/*############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.MOLASSES 
#
#    Copyright (C) 2015-2020  
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

#include <prototypes_LJC.h>

int SET_FLOW_PARAMS(
Inputs *In, 
VentArr *vent, 
double *gridinfo, 
DataCell **grid)
{
	float	log_min;
	float	log_max;
	int i, j;
  
	if (In->min_residual > 0 && 
	In->max_residual > 0 && 
	In->max_residual >= In->min_residual) 
	{	
		if (In->log_mean_residual > 0 && In->log_std_residual > 0) 
		{
			log_min = log10(In->min_residual);
			log_max = log10(In->max_residual);
			while (vent->residual = 
					(double) gennor( (float) In->log_mean_residual, (float) In->log_std_residual ),vent->residual > log_max || vent->residual < log_min);
			vent->residual = pow(10, vent->residual); 
		}
		else vent->residual = (double) genunf( (float) In->min_residual, (float) In->max_residual );
      fprintf(stderr, "Flow residual: %0.2f (meters)\n", vent->residual);
	}
	else vent->residual = In->residual;

	if (In->min_total_volume > 0 && 
	In->max_total_volume > 0 && 
	In->max_total_volume >= In->min_total_volume) 
	{
		if (In->log_mean_volume > 0 && In->log_std_volume > 0) 
		{
				log_min = log10(In->min_total_volume);
				log_max = log10(In->max_total_volume);
				while (vent->volumeToErupt = 
						(double) gennor ( (float) In->log_mean_volume, (float) In->log_std_volume ),
						vent->volumeToErupt > log_max || vent->volumeToErupt < log_min);
				vent->volumeToErupt = pow(10, vent->volumeToErupt); 
		}
		else vent->volumeToErupt = (double) genunf ( (float) In->min_total_volume, (float) In->max_total_volume );
		fprintf(stderr, "Total lava volume: %0.2g (cubic meters)\n", vent->volumeToErupt);
		vent->currentvolume = vent->volumeToErupt;
	}

	if (In->min_pulse_volume > 0 && 
		In->max_pulse_volume > 0 && 
		In->max_pulse_volume >= In->min_pulse_volume) 
	{
		vent->pulsevolume = 
		(double) genunf ( (float) In->min_pulse_volume, (float) In->max_pulse_volume );
		fprintf(stderr, "Flow pulse volume: %0.2g (cubic meters)\n", vent->pulsevolume);
	}

	/*Write residual value into 2D Global Data Array*/
	for(i=0; i < gridinfo[4]; i++) 
	{
		for(j=0; j < gridinfo[2]; j++) 
		{
			grid[i][j].residual = vent->residual;
		}
	}
	return(0);
}
