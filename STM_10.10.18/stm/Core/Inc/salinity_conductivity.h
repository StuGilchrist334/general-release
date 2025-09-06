#ifndef SALINITY_CONDUCTIVITY_H_
#define SALINITY_CONDUCTIVITY_H_

double calculate_salinity(double peak_pot, double temp, int scan_type);
double calculate_specific_conductivity(double salinity);
double calculate_actual_conductivity(double specidic_conductivity, double temp, int scan_type);

#endif /* SALINITY_CONDUCTIVITY_H_ */
