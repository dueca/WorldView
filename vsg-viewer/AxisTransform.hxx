/* ------------------------------------------------------------------   */
/*      item            : AxisTransform.hxx
        made by         : rvanpaassen
        date            : Mon Nov 22 2010
        category        : header file
        description     : convert aircraft axes to vsg objects
        changes         : Mon Nov 22 2010 copied from plib version
        language        : C++
*/

#pragma once

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

    Vehicle Models:    -y front
                       -x right
                       z  up

    Scenery models have the same orientation, drawn on the x y plane, with
    z indicating height.

    To avoid confusion with the model worlds, the base viewpoint is
    rotated, to align x/north in aerospace, with -y in model space.

    Transformations made to models:

    aerospace                         model space
    +x                                -y
    +y                                -x
    +z                                -z

    Quaternion dueca:  w, x, y, z
    Quaternion vsg:    x, y, z, w
*/
namespace vsgviewer {

  template <class CLS>
  class vrange {
    typename CLS::const_pointer base;
    size_t len;
  public:

    // needed for access
    typedef typename CLS::value_type value_type;
    typedef typename CLS::reference reference;
    typedef typename CLS::const_reference const_reference;
    typedef typename CLS::size_type size_type;

    vrange(const CLS &vect, size_t pos, size_t len) :
      base(vect.data() + pos),
      len(len)
    {
      if (pos+len > vect.size()) {
        throw std::out_of_range("vrange error reference exceeds");
      }
    }

    const_reference operator[](size_type pos) const
    {
      if (pos >= len) {
        throw std::out_of_range("vrange index exceeded");
      }
      return * (base+pos);
    }
  };

  /** Generate an vsg orientation quaternion from quaternion coordinates
      in the aircraft reference frame */
  template <typename T>
   inline vsg::t_quat<T> vsgQuat(const T q0, const T q1,
                                       const T q2, const T q3)
  {
    return vsg::t_quat<T>(q2, q1, -q3, q0);
  }

  /** Generate an vsg orientation quaternion from quaternion
      vector/array in the aircraft reference frame */
  template <typename T>
   inline vsg::t_quat<typename T::value_type> vsgQuat(const T& q)
  {
    return vsgQuat(q[0], q[1], q[2], q[3]);
  }

  template <typename T>
   inline vsg::t_quat<typename T::value_type> vsgQuatInv(const T &q)
  {
    return vsg::t_quat<typename T::value_type>(-q[2], -q[1], q[3], q[0]);
  }

  /** Calculate from a position aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down,
      to the vsg world. */
  template <typename T>
  static inline vsg::t_vec3<T> vsgPos(const T x, const T y, const T z)
  {
    return vsg::t_vec3<T>(-y, -x, -z);
  }

  /** Calculate from a position aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down,
      to the vsg world. */
  template <typename T>
   inline vsg::t_vec3<typename T::value_type> vsgPos(const T xyz)
  {
    return vsg::t_vec3<typename T::value_type>(-xyz[1], -xyz[0], -xyz[2]);
  }

  /** Switch the scaling to the proper dimensions */
  template <typename T>
   inline vsg::t_vec3<T> vsgScale(const T x, const T y, const T z)
  {
    return vsg::t_vec3(y, x, z);
  }

  /** Switch the scaling to the proper dimensions */
  template <typename T>
   inline vsg::t_vec3<typename T::value_type> vsgScale(const T xyz)
  {
    return vsg::t_vec3<typename T::value_type>(xyz[1], xyz[0], xyz[2]);
  }

  template <typename T>
   inline vsg::t_mat4<T> vsgRotation(const T phi, const T tht, const T psi)
  {
    // not sure abt order of axes and angles
    static const vsg::t_vec3<T> xax( 0.0, -1.0,  0.0);
    static const vsg::t_vec3<T> yax(-1.0,  0.0,  0.0);
    static const vsg::t_vec3<T> zax( 0.0,  0.0, -1.0);
    return vsg::rotate(phi, xax) * vsg::rotate(tht, yax) * vsg::rotate(psi, zax);
  }

};

