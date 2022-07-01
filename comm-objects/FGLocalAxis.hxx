/* ------------------------------------------------------------------   */
/*      item            : FGLocalAxis.hxx
        made by         : rvanpaassen
        date            : Mon Jun 29 12:58:26 2009
	category        : header file 
        description     : convert aircraft axes to FlightGear lat-lon-alt
	changes         : Mon Jun 29 12:58:26 2009 first version
        language        : C++
*/

#ifndef FGLocalAxis_hxx
#define FGLocalAxis_hxx

#include <mtl/mtl.h>

typedef mtl::matrix<double, mtl::rectangle<>, 
		    mtl::dense<>, mtl::row_major>::type Matrix;
typedef mtl::external_vec<double> VectorE;

/** Glue object; Axis transformation from a user's reference frame to
    Flight Gear stuff. */
struct FGLocalAxis
{
  /** Local Radius meridian */
  double RM;
  
  /** Local Radius parallel */
  double RP;

  /** Conversion to north-up system */
  Matrix conv;

  /** inverse cosine of latitude */
  double invCosLat;

  /** Constructor. Assumes a world coordinate system with 
      <ol>
      <li> x pointing psi_zero, y pointing psi_zero+90, z pointing down 
      </ol>
      \param lat_zero Geodetic latitude of origin 
      \param lon_zero Longitude of origin 
      \param psi_zero Local axis direction */
  FGLocalAxis(double lat_zero = 0.0, double lon_zero = 0.0,
	      double psi_zero = 0.0); 
};
  

#endif  
