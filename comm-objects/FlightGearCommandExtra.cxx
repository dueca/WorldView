#include "FGLocalAxis.hxx"
#include "ObjectMotion.hxx"
typedef mtl::external_vec<const double> constVectorE;

void FlightGearCommand::setCommand(const ObjectMotion& obj, 
				   const FGLocalAxis& ax)
{
  setCommand(obj.xyz, obj.getPhi(), obj.getTht(), obj.getPsi(), ax);
}

void FlightGearCommand::setCommand(const double xyz[3], const double phi, 
				   const double tht, const double psi,
				   const FGLocalAxis& ax)
{
  // first convert to the north-up system
  double xy2_data[2];
  constVectorE xy(xyz, 2);
  VectorE xy2(xy2_data, 2);
  mult(ax.conv, xy, xy2);
  // latitude in degrees, from meridian radius, and convert to degrees
  latitude = xy2[0] / (ax.RM - xyz[2]);
  
  // longitude in degrees
  longitude = xy[1] / (ax.RP - xyz[2]) * ax.invCosLat;

  // altitude in feet
  altitude = -0.3048 * xyz[2];

  // euler angles
  roll = phi * 180.0 / M_PI;
  pitch = tht * 180.0 / M_PI;
  heading = psi * 180.0 / M_PI;
}
  
void FlightGearCommand::setCommand(const double xyz[3], const double quat[4],
				   const FGLocalAxis& ax)
{
  double phi = atan2(2.0*double(quat[2]*quat[3] + 
				quat[1]*quat[0]), 
		     double(quat[0]*quat[0] - 
			    quat[1]*quat[1] - 
			    quat[2]*quat[2] + 
			    quat[3]*quat[3])); 
  double tht = asin(-2.0*double(quat[1]*quat[3] -
				quat[2]*quat[0]));
  double psi = atan2(2.0*double(quat[1]*quat[2] + 
			    quat[3]*quat[0]), 
		     double(quat[0]*quat[0] + 
			    quat[1]*quat[1] - 
			    quat[2]*quat[2] - 
			    quat[3]*quat[3]));
  setCommand(xyz, phi, tht, psi, ax);
}
