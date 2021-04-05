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

#include "include/prototypes_LJC2.h"

/*************************************
Module: PULSE
If volume remains to be erupted, deliver a pre-defined pulse of magma to 
vent cell and subtract this pulse from the vent cell's volume. Return the
total remaining volume through a pointer to the volumeRemaining variable.

INPUTS:
ActiveList *actList
Lava_flow *active_flow
DataCell **grid
double *volumeRemaining
double *gridinfo

Algorithm:
	  If more lava remains to be erupted:
	    Calculate thickness to deliver to vent
	    Add thickness to vent cell, subtract volume from remaining volume in cell
	    Calculate total volume remaining, exit
	  If no lava remains to be erupted:
	    Exit without doing anything
	
gridinfo elements used in this module:
[1] w-e pixel resolution
[5] n-s pixel resolution (negative value)

Calculate thickness of lava to deliver 
based on pulse volume and grid resolution
*****************************************/
     
void PULSE(
/*unsigned int *aCT,*/
ActiveList *actList,
/* VentArr *vent, */
Lava_flow *active_flow,
DataCell **grid,
double *volumeRemaining,
double *gridinfo)
{

	double pulseThickness = 0.0; /*Pulse Volume divided by data grid resolution*/
	double pulsevolume;
	
	pulsevolume = active_flow->pulsevolume;
  
	if (*volumeRemaining > 0.0) {
		if (pulsevolume > active_flow->currentvolume) pulsevolume = active_flow->currentvolume;	
		pulseThickness = pulsevolume / (gridinfo[1] * gridinfo[5]);
 
		active_flow->currentvolume -= pulsevolume; /* Subtract pulse volume from flow's total magma budget */
		*volumeRemaining = active_flow->currentvolume;
		
		/* If the flow has a thickness of lava greater than it's residual, then it has lava to give
		   so put it on the active list.		   
		 */
		 
		 /* grid[active_flow->(source+i)->row][active_flow->(source+i)->col].eff_elev += pulseThickness */
		 /* assign the current vent cell, it will be 0 (first) on the active list; only one vent can erupt at a time */
		 
		 grid[actList->row][actList->col].eff_elev += pulseThickness; 	
	}
#ifdef PRINT
	fprintf (stderr,  
		"[PULSE] pulse thickness = %f /(%f * %f) = %f\n",
		pulsevolume, gridinfo[1], gridinfo[5], pulseThickness);
		fflush(stderr);
#endif		
	return; /* simply RETURN, if no more volume to erupt */
}
