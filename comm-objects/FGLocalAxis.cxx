/* ------------------------------------------------------------------   */
/*      item            : FGLocalAxis.cxx
        made by         : rvanpaassen
        date            : Mon Jun 29 12:58:26 2009
	category        : header file 
        description     : convert aircraft axes to Ogre objects
	changes         : Mon Jun 29 12:58:26 2009 first version
        language        : C++
*/

#include "FGLocalAxis.hxx"
#include <cmath>

/** Elliptic world definition */
static const double a = 6378137.0;
static const double f = 1/298.2572;
static const double esqr = f*(2.0 - f);

FGLocalAxis::FGLocalAxis(double lat_zero, double lon_zero, double psi_zero) :
  invCosLat(1/cos(lat_zero))
{
  // Kayton & Fried, p 25
  
  // Meridian radius of curvature
  RM = a*(1.0 + esqr*(1.5*sin(lat_zero)*sin(lat_zero) - 1));
  // Prime radius of curvature
  RP = a*(1.0 + esqr*0.5*sin(lat_zero)*sin(lat_zero));
  
  // conversion
  conv(0,0) =  cos(psi_zero);
  conv(0,1) = -sin(psi_zero);
  conv(1,0) =  sin(psi_zero);
  conv(1,1) =  cos(psi_zero);
}

