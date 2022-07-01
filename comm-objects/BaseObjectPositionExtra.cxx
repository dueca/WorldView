#include <cmath>
#include <cmath>

#define __CUSTOM_COMPATLEVEL_110

static const int qX = 1;
static const int qY = 2;
static const int qZ = 3;
static const int qW = 0;


void BaseObjectPosition::setquat(double phi, double tht, double psi)
{
  attitude_q[qW] = cos(0.5*phi)*cos(0.5*tht)*cos(0.5*psi) +
    sin(0.5*phi)*sin(0.5*tht)*sin(0.5*psi);
  attitude_q[qX] = sin(0.5*phi)*cos(0.5*tht)*cos(0.5*psi) -
    cos(0.5*phi)*sin(0.5*tht)*sin(0.5*psi); 
  attitude_q[qY] = cos(0.5*phi)*sin(0.5*tht)*cos(0.5*psi) +
    sin(0.5*phi)*cos(0.5*tht)*sin(0.5*psi); 
  attitude_q[qZ] = cos(0.5*phi)*cos(0.5*tht)*sin(0.5*psi) -
    sin(0.5*phi)*sin(0.5*tht)*cos(0.5*psi);
}

double BaseObjectPosition::getPhi() const
{
  return atan2(2.0*double(attitude_q[2]*attitude_q[3] + 
			  attitude_q[1]*attitude_q[0]), 
	       double(attitude_q[0]*attitude_q[0] - 
		      attitude_q[1]*attitude_q[1] - 
		      attitude_q[2]*attitude_q[2] + 
		      attitude_q[3]*attitude_q[3]));
}

double BaseObjectPosition::getTht() const
{
  return asin(-2.0*double(attitude_q[1]*attitude_q[3] -
			  attitude_q[2]*attitude_q[0]));
}

double BaseObjectPosition::getPsi() const
{
  return atan2(2.0*double(attitude_q[1]*attitude_q[2] + 
			  attitude_q[3]*attitude_q[0]), 
	       double(attitude_q[0]*attitude_q[0] + 
		      attitude_q[1]*attitude_q[1] - 
		      attitude_q[2]*attitude_q[2] - 
		      attitude_q[3]*attitude_q[3]));
}
