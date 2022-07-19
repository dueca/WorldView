/* ------------------------------------------------------------------   */
/*      item            : AxisTransform.hxx
        made by         : rvanpaassen
        date            : Mon Nov 22 2010
        category        : header file 
        description     : convert aircraft axes to vsg objects
        changes         : Mon Nov 22 2010 copied from plib version
        language        : C++
*/

#ifndef AxisTransform_hxx
#define AxisTransform_hxx

#include <vsg/all.h>
#include <RvPQuat.hxx>

/** Glue object; Axis transformation from a user's reference frame to
    plib/ssg position and attitude (quaternion) objects. 

    My representation; x north
                       y east
		       z down
		       psi: rotation about z
		       tht: rotation about y
		       phi: rotation about x

    Vsg;               x right on screen
                       y up on screen
		       z towards you
		       psi: rotation about z axis
		       tht: rotation about x axis
		       phi: rotation about -z axis
*/
struct AxisTransform
{
  /** Generate an vsg orientation quaternion from quaternion coordinates
      in the aircraft reference frame */
  template <typename T>
  static inline vsg::quat vsgQuat(const T q0, const T q1, 
				  const T q2, const T q3)
  { 
    vsg::quat quat(q2, q1, -q3, q0);
    return quat; 
  }

  /** Generate an vsg orientation quaternion from quaternion
      vector/array in the aircraft reference frame */
  template <typename T>
  static inline vsg::quat vsgQuat(T q) 
  {
    return vsgQuat(q[0], q[1], q[2], q[3]);
  }

  /** Calculate from a position aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down, 
      to the vsg world. */
  template <typename T>
  static inline vsg::t_vec3<T> vsgPos(const T x, const T y, const T z)
  { 
    vsg::t_vec3<T> vec(y, x, -z);
    return vec; 
  }

  /** Calculate from a position aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down, 
      to the vsg world. */
  template <typename T>
  static inline vsg::t_vec3<T> vsgPos(const T xyz)
  { 
    return vsgPos(xyz[0], xyz[1], xyz[2]); 
  }

  /** Switch the scaling to the proper dimensions */
  template <typename T>
  static inline vsg::t_vec3<T> vsgScale(const T xyz)
  { 
    return vsg::t_vec3<T>(xyz[1], xyz[0], xyz[2]); 
  }

  template <typename T>
  static inline vsg::t_mat3<T> vsgRotation(const T phi, const T tht, const T psi)
  {
    // not sure abt order of axes and angles
    static const vsg::t_vec3<T> xax( 0.0,  1.0,  0.0);
    static const vsg::t_vec3<T> yax( 1.0,  0.0,  0.0);
    static const vsg::t_vec3<T> zax( 0.0,  0.0, -1.0);
    vsg::t_mat3<T> result;
    result.makeRotate(phi, xax, tht, yax, psi, zax);
    return result;
  }  
};



#endif
