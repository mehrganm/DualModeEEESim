#include "IPPPacketGenerator.h"

void IPPPacketGenerator::run()
{
	curr_sq = start_sq;
	running = true;
	Packet * newPacket;

	double   ipp_rv;              // IPP random variable
	double   temp, temp1;         // Variables needed for IPP to H2 conversion
	double   lambda1, lambda2;    // Variables needed for IPP to H2 conversion
	double   pi1;                 // Variable needed for IPP to H2 conversion

    rand_val(rand_seed);

	// Conversion from IPP to H2
	temp = (lambda + alpha + beta);
	temp1 = (4.0 * lambda * beta);
	lambda1 = (0.5*(temp + sqrt(temp*temp - temp1)));
	lambda2 = (0.5*(temp - sqrt(temp*temp - temp1)));
	pi1 = (lambda - lambda2)/(lambda1 - lambda2);

    create(my_name.c_str());

	printf ("%.3f: %s: Packet %ld generated\n", clock*1.0e+6, my_name.c_str(), curr_sq);
	newPacket = new Packet(clock, curr_sq, 12000, 0, my_name + std::string(":") + std::to_string(curr_sq));
	curr_sq++;
	link->processPacket(newPacket);

	while (running)
	{
		if (rand_val(0) < pi1)
		  ipp_rv = expon(1.0 / lambda1);
		else
		  ipp_rv = expon(1.0 / lambda2);

		hold(ipp_rv);

		printf ("%.3f: %s: Packet %ld generated\n", clock*1.0e+6, my_name.c_str(), curr_sq);
		newPacket = new Packet(clock, curr_sq, 12000, 0, my_name + std::string(":") + std::to_string(curr_sq));
		link->processPacket(newPacket);
		curr_sq++;
	}
}

//=============================================================================
//=  Function to generate exponentially distributed RVs                       =
//=    - Input:  x (mean value of distribution)                               =
//=    - Output: Returns with exponential RV                                  =
//=============================================================================
double IPPPacketGenerator::expon(double x)
{
  double z;     // Uniform random number from 0 to 1

  // Pull a uniform RV (0 < z < 1)
  do
  {
    z = rand_val(0);
  }
  while ((z == 0) || (z == 1));

  return(-x * log(z));
}

//=========================================================================
//= Multiplicative LCG for generating uniform(0.0, 1.0) random numbers    =
//=   - x_n = 7^5*x_(n-1)mod(2^31 - 1)                                    =
//=   - With x seeded to 1 the 10000th x value should be 1043618065       =
//=   - From R. Jain, "The Art of Computer Systems Performance Analysis," =
//=     John Wiley & Sons, 1991. (Page 443, Figure 26.2)                  =
//=========================================================================
double IPPPacketGenerator::rand_val(int seed)
{
  const long  a =      16807;  // Multiplier
  const long  m = 2147483647;  // Modulus
  const long  q =     127773;  // m div a
  const long  r =       2836;  // m mod a
  static long x;               // Random int value
  long        x_div_q;         // x divided by q
  long        x_mod_q;         // x modulo q
  long        x_new;           // New x value

  // Set the seed if argument is non-zero and then return zero
  if (seed > 0)
  {
    x = seed;
    return(0.0);
  }

  // RNG using integer arithmetic
  x_div_q = x / q;
  x_mod_q = x % q;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0)
    x = x_new;
  else
    x = x_new + m;

  // Return a random value between 0.0 and 1.0
  return((double) x / m);
}
