/* ------------------------------------------------------------------   */
/*      item            : AxisTransform.hxx
        made by         : rvanpaassen
        date            : Mon Nov 22 2010
        category        : header file 
        description     : convert aircraft axes to osg objects
        changes         : Mon Nov 22 2010 copied from plib version
        language        : C++
*/

#ifndef AxisTransform_hxx
#define AxisTransform_hxx

#include <osg/Vec3d>
#include <osg/Quat>
#include <RvPQuat.hxx>

/** Glue object; Axis transformation from a user's reference frame to
    plib/ssg position and attitude (quaternion) objects. 

    My representation; x north
                       y east
		       z down
		       psi: rotation about z
		       tht: rotation about y
		       phi: rotation about x

    Osg;               x right on screen
                       y up on screen
		       z towards you
		       psi: rotation about z axis
		       tht: rotation about x axis
		       phi: rotation about -z axis
*/
struct AxisTransform
{
  /** Generate an osg orientation quaternion from quaternion coordinates
      in the aircraft reference frame */
  template <typename T>
  static inline osg::Quat osgQuat(const T q0, const T q1, 
				  const T q2, const T q3)
  { 
    osg::Quat quat(q2, q1, -q3, q0);
    return quat; 
  }

  /** Generate an osg orientation quaternion from quaternion
      vector/array in the aircraft reference frame */
  template <typename T>
  static inline osg::Quat osgQuat(T q) 
  {
    return osgQuat(q[0], q[1], q[2], q[3]);
  }

  /** Calculate from a position aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down, 
      to the osg world. */
  template <typename T>
  static inline osg::Vec3d osgPos(const T x, const T y, const T z)
  { 
    osg::Vec3d vec(y, x, -z);
    return vec; 
  }

  /** Calculate from a position aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down, 
      to the osg world. */
  template <typename T>
  static inline osg::Vec3d osgPos(const T xyz)
  { 
    return osgPos(xyz[0], xyz[1], xyz[2]); 
  }

  /** Switch the scaling to the proper dimensions */
  template <typename T>
  static inline osg::Vec3d osgScale(const T xyz)
  { 
    return osg::Vec3d(xyz[1], xyz[0], xyz[2]); 
  }

  template <typename T>
  static inline osg::Matrixd osgRotation(const T phi, const T tht, const T psi)
  {
    // not sure abt order of axes and angles
    static const osg::Vec3d xax( 0.0,  1.0,  0.0);
    static const osg::Vec3d yax( 1.0,  0.0,  0.0);
    static const osg::Vec3d zax( 0.0,  0.0, -1.0);
    osg::Matrixd result;
    result.makeRotate(phi, xax, tht, yax, psi, zax);
    return result;
  }  
};



#endif
