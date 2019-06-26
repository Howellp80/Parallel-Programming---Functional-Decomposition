/******************************************************************************
* Parker Howell
* Project 4 - Functional Decomposition
* 5-14-18
* This project will use parallelism, not for speeding data computation, but for 
* programming convenience. This is a month-by-month simulation of a 
* grain-growing operation. The amount the grain grows is affected by the 
* temperature, amount of precipitation, and the number of "graindeer" around to
* eat it. The number of graindeer depends on the amount of grain available to 
* eat. 
*******************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <iostream>





// constants
const float GRAIN_GROWS_PER_MONTH =			10.0;
const float ONE_DEER_EATS_PER_MONTH =		0.4;

const float AVG_PRECIP_PER_MONTH =			6.0;	// average
const float AMP_PRECIP_PER_MONTH =			6.0;	// plus or minus
const float RANDOM_PRECIP =					2.0;	// plus or minus noise

const float AVG_TEMP =							50.0;	// average
const float AMP_TEMP =							20.0;	// plus or minus
const float RANDOM_TEMP =						10.0;	// plus or minus noise

const float MIDTEMP =							40.0;
const float MIDPRECIP =							10.0;



// state variables
int	NowYear;				// 2017 - 2022
int	NowMonth;			// 0 - 11

float	NowPrecip;			// inches of rain per month
float	NowTemp;				// temperature this month
float	NowHeight;			// grain height in inches
int 	NowNumHunters;		// number of hunters in the current population
int	NowNumDeer;		// number of deer in the current population
unsigned int seed = 0;	// seed value for rand



float Ranf( unsigned int *seedp,  float low, float high ){
	float r = (float) rand_r( seedp );     // 0 - RAND_MAX

	return(low + r * (high - low) / (float)RAND_MAX);
}




int Ranf( unsigned int *seedp, int ilow, int ihigh ){
	float low = (float)ilow;
	float high = (float)ihigh + 0.9999f;

	return (int)(Ranf(seedp, low, high));
}




// generates semi random climate states
void setState(){
	float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

	float temp = AVG_TEMP - AMP_TEMP * cos( ang );
	NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

	float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
	NowPrecip = precip + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
	if( NowPrecip < 0. )
		NowPrecip = 0.;
}




// returns the square of argument x
float SQR( float x ){
	return x*x;
}




// determines and returns the next Grain Hieght value
float getNextGH(){
	float grainHeight = NowHeight;

	// how much the temperature will affect growth
	float tempFactor = exp( -SQR((NowTemp - MIDTEMP) / 10.));
	// how much the rain will affect growth
	float precipFactor = exp( -SQR((NowPrecip - MIDPRECIP) / 10.));

	grainHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
	grainHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
	
	if(grainHeight < 0.)
		grainHeight = 0.;

	return grainHeight;
}




// determines and returns the next GrainDeer count value
int getNextGDC(){
	int deerCount = NowNumDeer;

	float deerEat = deerCount * ONE_DEER_EATS_PER_MONTH;

	// if there is enought grain to support more deer
	if(NowHeight > deerEat)
		deerCount++;
	// else if there isnt enought food
	else {
		deerCount--;
		if (deerCount < 0)
			deerCount = 0;
	}

	return deerCount;
}



// determines and returns the next Hunters count value
int getNextHC(){
	int hunters;

	// set hunters based on deer population
	if(NowNumDeer > 15){
		hunters = 3;
		//std::cout << "set 3\n";
	}
	else if(NowNumDeer > 10){
		hunters = 2;
		//std::cout << "set 2\n";
	}
	else if(NowNumDeer > 5){
		hunters = 1;
		//std::cout << "set 1\n";
	}
	else{
		hunters = 0;
	}

	// reduce hunters if too cold or hot
	if(NowTemp < 40 || NowTemp > 85){
		hunters--;
	}

	// reduce hunters if too rainy
	if(NowPrecip > 11){
		hunters--;
	}

	// check for negative value
	if(hunters < 0){
		hunters = 0;
	}

	return hunters;
}




////////////////////////////  Sections Here  ////////////////////////////////

// section for GrainDeer
void GrainDeer(){
	while( NowYear < 2023 ){

		//float nextGrainHeight = getNextGH();
		int nextDeerCount = getNextGDC();
		nextDeerCount -= NowNumHunters;

		// DoneComputing barrier:
		#pragma omp barrier
		
		// set global GrainDeer count state
		NowNumDeer = nextDeerCount;

		// DoneAssigning barrier:
		#pragma omp barrier

		// DonePrinting barrier:
		#pragma omp barrier
	}
}




// section for Grain
void Grain(){
	while( NowYear < 2023 ){

		float nextGrainHeight = getNextGH();

		// DoneComputing barrier:
		#pragma omp barrier
		
		// set global GrainDeer count state
		NowHeight = nextGrainHeight;

		// DoneAssigning barrier:
		#pragma omp barrier

		// DonePrinting barrier:
		#pragma omp barrier
	}
}




// section for Watcher
void Watcher(){
	while( NowYear < 2023 ){

		// DoneComputing barrier:
		#pragma omp barrier

		// DoneAssigning barrier:
		#pragma omp barrier

		// output data
		int yearNum = NowYear - 2016;
		
		std::cout  << NowPrecip << "\t" 
					  << NowTemp << "\t" 
					  << NowHeight << "\t" 
					  << NowNumHunters << "\t"
					  << NowNumDeer << std::endl;

		// increment month
		NowMonth++;
		if(NowMonth >= 12){
			NowMonth = 0;
			NowYear++;
		}

		// with a new date, update the climate state
		setState();

		// DonePrinting barrier:
		#pragma omp barrier
	}
}////////////////////////////  End Sections  //////////////////////////////




// section for Hunters
void Hunters(){
	while( NowYear < 2023 ){

		int nextHunterCount = getNextHC();

		// DoneComputing barrier:
		#pragma omp barrier
		
		// set global Hunters count state
		NowNumHunters = nextHunterCount;

		// DoneAssigning barrier:
		#pragma omp barrier

		// DonePrinting barrier:
		#pragma omp barrier
	}
}






int main(int argc, char* argv[]){
	
	// starting date and time:
	NowMonth =    0;
	NowYear  = 2017;

	// starting state:
	NowNumDeer = 1;
	NowHeight =  1.;
	NowNumHunters = 0;

	// set starting climate state
	setState();


	// create our thread pool
	omp_set_num_threads( 4 );	// same as # of sections
	
	// divy up sections to threads
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			GrainDeer();
		}

		#pragma omp section
		{
			Grain();
		}

		#pragma omp section
		{
			Watcher();
		}

		#pragma omp section
		{
			Hunters();	
		}
	}       // implied barrier -- all functions must return in order
		// to allow any of them to get past here

	std::cout << "\n";
	return(0);
}



