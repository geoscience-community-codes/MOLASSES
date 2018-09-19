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

#include "include/prototypes_LJC.h"

/*************************************
Module: PULSE
If volume remains to be erupted, delivers a pre-defined pulse of magma to 
vent cell and subtracts this pulse from the vent cell's volume. Returns the
total remaining volume through a pointer to the volumeRemaining variable.

INPUTS:
Automata *actList
VentArr *vent
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
Automata *actList,
VentArr *vent,
DataCell **grid,
double *volumeRemaining,
double *gridinfo)
{

	double pulseThickness = 0.0; /*Pulse Volume divided by data grid resolution*/
	double pulsevolume;
	
	pulsevolume = vent->pulsevolume;

	if (*volumeRemaining > 0.0) {
		if (pulsevolume > vent->currentvolume) pulsevolume = vent->currentvolume;	
		pulseThickness = pulsevolume / (gridinfo[1] * gridinfo[5]);
		grid[actList->row][actList->col].eff_elev += pulseThickness;
		vent->currentvolume -= pulsevolume; /* Subtract pulse volume from vent's total magma budget */
		*volumeRemaining = vent->currentvolume;
	}
	
#ifdef PRINT1 
		fprintf (stderr,  
		"\nOUT PULSE pulse thickness = %f /(%f * %f) = %f\n",
		pulsevolume, gridinfo[1], gridinfo[5], pulseThickness);
#endif

/*	fflush(stderr);*/
	return; /* simply RETURN, if no more volume to erupt */
}
