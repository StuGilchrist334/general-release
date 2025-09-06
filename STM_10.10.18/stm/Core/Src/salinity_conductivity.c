#include "salinity_conductivity.h"
#include "calculate_ph.h"

double calculate_salinity(double peak_pot, double temp, int scan_type)
{
	//working equation is a quadratic for lnS vs ((iref+800) / 1400)

	double a_3 = 570.63, a_2 = -1972.6, a_1 = 2233.2, a_0 = -827.12, iref_zero = 800, iref_star;

	iref_star = (peak_pot + iref_zero) / 1400;

	double ln_salinity = a_3*iref_star*iref_star*iref_star + a_2*iref_star*iref_star + a_1*iref_star +a_0;

	double salinity = exp(ln_salinity);

	if (salinity < 0.0)
		salinity = 0.0; //this enables a pH to be calculated

	if ((salinity > 7.5) && !ocean_scan)
	{
		salinity_out_of_range = true;
		salinity = SALINITY_DEFAULT_OUT_OF_RANGE_VALUE;
	}
	if ((salinity > 7.5) && ocean_scan)
	{
		salinity_out_of_range = true;
	}
	if((salinity < 1.0) && ocean_scan)
	{
		salinity_out_of_range = true;
		salinity = SALINITY_DEFAULT_OUT_OF_RANGE_VALUE;
	}

	return salinity;
}

double calculate_specific_conductivity(double salinity)
{
	double a = -0.0284;
	double b = 1.9319;
	double c = 0.049;

	double specific_conductivity = a*salinity*salinity + b*salinity +c;

	return specific_conductivity;
}

double calculate_actual_conductivity(double specific_conductivity, double temp, int scan_type)
{
	double alpha, alpha_f = 0.02, alpha_o = 0.025;

	if(ocean_scan)
		alpha = alpha_o;
	else
		alpha = alpha_f;

	double actual_conductivity = specific_conductivity * (1.0 + alpha*(temp - 25.0));

	return actual_conductivity;
}
