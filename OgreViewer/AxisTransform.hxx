/* ------------------------------------------------------------------   */
/*      item            : AxisTransform.hxx
        made by         : rvanpaassen
        date            : Mon Jun 29 12:58:26 2009
	category        : header file 
        description     : convert aircraft axes to Ogre objects
	changes         : Mon Jun 29 12:58:26 2009 first version
        language        : C++
*/

#ifndef AxisTransform_hxx
#define AxisTransform_hxx

#include <Ogre.h>

/** Glue object; Axis transformation from a user's reference frame to
    Ogre position and attitude (quaternion) objects. */
struct AxisTransform
{
  /** Generate an ogre rotation matrix from three euler angles in the
      a/c reference frame */
  template <typename T>
  static inline Ogre::Matrix3 ogreRotation(T phi, T tht, T psi)
  { Ogre::Matrix3 rot; 
    rot.FromEulerAnglesYXZ
      (Ogre::Degree(-psi), Ogre::Degree(tht), Ogre::Degree(-phi)); 
    return rot; }

  /** Calculate from a position in aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down, 
      to the ogre world; 

      Ogre coordinates are x to the right, y up, z
      pointing towards the viewer. 
      
      Mapping is so that aircraft x becomes Ogre's -z
                         aircraft y becomes Ogre's x
			 aircraft z becomes Ogre's -y
  */
  template <typename T>
  static inline Ogre::Vector3 ogrePosition(T x, T y, T z)
  { return Ogre::Vector3(y, -z, -x); }

 /** Calculate from a position in aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down, 
      to the ogre world; 

      Ogre coordinates are x to the right, y up, z
      pointing towards the viewer. 
      
      Mapping is so that aircraft x becomes Ogre's -z
                         aircraft y becomes Ogre's x
			 aircraft z becomes Ogre's -y
  */
  template <typename T>
  static inline Ogre::Vector3 ogrePosition(const T& xyz)
  { return Ogre::Vector3(xyz[1], -xyz[2], -xyz[0]); }
  

  /** Calculate from a rotation in aircraft earth-fixed, earth
      centered reference frame, x north, y east, z down, 
      to the ogre world. This takes the aircraft yaw, pitch and roll
      angles and applies them to an Ogre frame. They are applied as -
      rotation about Ogre Y axis, rotation around Ogre X axis, and -
      rotation around Ogre z axis
  */
  template <typename T>
  static inline Ogre::Quaternion ogreQuaternion(T phi, T tht, T psi)
  { Ogre::Matrix3 rot = ogreRotation(phi, tht, psi);
    return Ogre::Quaternion(rot); }

  /** Convert a rotation quaternion from the aircraft axis system to
      Ogre. Rotation size is constant, axes have to be converted. */
  template <typename T>
  static inline Ogre::Quaternion ogreQuaternion(const T& acq)
  { return Ogre::Quaternion(acq[0], acq[2], -acq[3], -acq[1]); }
    
};

#endif  
