/* ------------------------------------------------------------------   */
/*      item            : AxisTransform.hxx
        made by         : rvanpaassen
        date            : Mon Jun 29 12:58:26 2009
        category        : header file 
        description     : convert aircraft axes to plib/ssg objects
        changes         : Fri Jan 29 12:58:26 2009 first version
        language        : C++
*/

#ifndef AxisTransform_hxx
#define AxisTransform_hxx

#include <plib/ssg.h>
#include <RvPQuat.hxx>

/** Glue object; Axis transformation from a user's reference frame to
    plib/ssg position and attitude (quaternion) objects. 

    My representation; x north
                       y east
		       z down
		       psi: rotation about z
		       tht: rotation about y
		       phi: rotation about x

    Plib;              x east
                       y up
		       z north
		       (create a model flying away from you)
		       psi: rotation about -y axis
		       tht: rotation about x axis
		       phi: rotation about -z axis
*/
struct AxisTransform
{
  /** Generate a plib coordinate struct from position and quaternion
      in the aircraft reference frame */
  template <typename T, typename Q>
  static inline sgCoord plibCoordinates(const T& xyz, const Q& quat) 
  {
    sgCoord res = {
      { sgFloat(xyz[1]), sgFloat(xyz[0]), sgFloat(-xyz[2]) }, 
      { sgFloat(-Q2psi(quat)*SG_RADIANS_TO_DEGREES), 
	  sgFloat(Q2tht(quat)*SG_RADIANS_TO_DEGREES), 
	  sgFloat(Q2phi(quat)*SG_RADIANS_TO_DEGREES)} };
    return res;
  }
};

#endif
