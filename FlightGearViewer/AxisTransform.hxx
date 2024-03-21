/* ------------------------------------------------------------------   */
/*      item            : FGLocalAxis.hxx
        made by         : rvanpaassen
        date            : Mon Jun 29 12:58:26 2009
        category        : header file
        description     : convert aircraft axes to Ogre objects
        changes         : Mon Jun 29 12:58:26 2009 first version
        language        : C++
*/

#ifndef FGLocalAxis_hxx
#define FGLocalAxis_hxx

#include "../comm-objects/RvPQuat.hxx"
#include <cmath>
#include <dueca-version.h>

#if DUECA_VERSION_NUM >= DUECA_VERSION(2, 0, 0)
#include <Eigen/Dense>
#define USING_EIGEN3
// a normal matrix, allocates its own storage
typedef Eigen::MatrixXd Matrix;
// a matrix that takes external storage
typedef Eigen::Map<
  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
  MatrixE;
// a matrix that takes external storage
typedef Eigen::Map<
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
  MatrixfE;
// a const matrix that takes external storage
typedef Eigen::Map<
  const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
  cMatrixfE;
// a const matrix that takes external storage
typedef Eigen::Map<
  const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
  cMatrixE;
// a normal vector, allocates its own storage
typedef Eigen::VectorXd Vector;
// a vector that takes external storage
typedef Eigen::Map<Eigen::VectorXd> VectorE;
// float vector, external storage
typedef Eigen::Map<Eigen::VectorXf> VectorfE;
// constant float vector, external storage
typedef Eigen::Map<const Eigen::VectorXf> cVectorfE;
// constant double vector, external storage
typedef Eigen::Map<const Eigen::VectorXd> cVectorE;
#else

#include <mtl/mtl.h>
typedef mtl::matrix<double, mtl::rectangle<>, mtl::dense<>,
                    mtl::row_major>::type Matrix;
typedef mtl::matrix<double, mtl::rectangle<>, mtl::dense<mtl::external>,
                    mtl::row_major>::type MatrixE;
typedef mtl::external_vec<double> VectorE;
typedef mtl::external_vec<float> VectorfE;
typedef mtl::external_vec<const double> cVectorE;
typedef mtl::dense1D<double> Vector;
#endif

struct EulerAngles;

/** Represent an orientation or rotation step with quaternion */
struct Orientation
{
  /** Quaternion Lambda parameter */
  double L;
  /** Quaternion lx */
  double lx;
  /** Quaternion ly */
  double ly;
  /** Quaternion lz */
  double lz;
  /** All coordinates as a vector */
  VectorE quat;

   /** Construction, phi, theta, psi */
  Orientation(const EulerAngles &angles);

  /** Construction from angle and axis */
  template <class V> Orientation(double angle, const V &axis);

  /** Default, null orientation */
  Orientation();

  /** Copy constructor */
  Orientation(const Orientation &o);

  /** Indexation operator */
  inline const double &operator[](const int i) const { return quat[i]; }

  /** Indexation operator */
  inline double &operator[](const int i) { return quat[i]; }

  /** Multiplication */
  inline Orientation operator*(const Orientation &o) const
  {
    Orientation res;
    QxQ(res, *this, o);
    return res;
  }
};
namespace std {
ostream &operator<<(ostream &os, const Orientation &c);
}

/** Euler angles describing an orientation */
struct EulerAngles
{
  /** roll angle */
  double phi;
  /** pitch angle */
  double tht;
  /** yaw angle */
  double psi;

    /** Construction, phi, theta, psi */
  EulerAngles(double phi, double tht, double psi);

  /** Construction from quaternion */
  EulerAngles(const Orientation &o);
};
namespace std {
ostream &operator<<(ostream &os, const EulerAngles &c);
}

class LocalAxis;
struct ECEF;
struct LatLonAlt;

/** Set of x, y, z Carthesian coordinates. */
struct Carthesian
{
  /** x coordinate */
  double x;
  /** y coordinate */
  double y;
  /** z coordinate */
  double z;
  /** All coordinates as a vector */
  VectorE xyz;

  /** Constructor, straightforward */
  Carthesian(double x = 0.0, double y = 0.0, double z = 0.0);

  /** Copy constructor */
  Carthesian(const Carthesian &o);

  /** Assignment !!! */
  Carthesian &operator=(const Carthesian &);
};

namespace std {
ostream &operator<<(ostream &os, const Carthesian &c);
}

/** ECEF coordinate set class. */
struct ECEF : public Carthesian
{
  /** Constructor, straightforward from x, y and z */
  ECEF(double x = 0.0, double y = 0.0, double z = 0.0);

    /** Constructor from lat, lon, altitude */
  ECEF(const LatLonAlt &lla);

  /** Copy constructor */
  ECEF(const ECEF &o);
};

/** Position on the WGS geoid in geodetic coordinates */
struct LatLonAlt
{
  /** Latitude */
  double lat;
  /** Longitude */
  double lon;
  /** Altitude */
  double alt;

    /** Constructor, straightforward from lat, lon and alt */
  LatLonAlt(double lat, double lon, double alt);

  /** Constructor from an ECEF object. Uses approximate calculation
      based on Bowring, 1976 */
  LatLonAlt(const ECEF &ecef);

  /** Meridian radius of curvature here */
  double RM() const;

  /** Prime radius of curvature here */
  double RP() const;

  /** Produce the rotation that converts local N-E-down vectors to
      global vectors */
  Orientation toGlobal(const double psi_zero = 0.0) const;
};

namespace std {
ostream &operator<<(ostream &os, const LatLonAlt &lla);
}

/** This class produces an "efficient" implementation of a local axis
    frame mapped onto ECEF or lat-lon-alt coordinates.

    The local axis frame is not exactly squarely mapped; some fudging
    is done (parabolic correction) to ensure that height above the
    terrain is approximately correct. Note that there are still deviations;
    60 NM lead to a height error of approximately 7 m (versus 900 m
    uncorrected!). This approximation is good enough for flying around
    a single airport. Otherwise, consider ECEF coordinates for your
    simulation and use those */
class LocalAxis
{
  /** Conversion matrix to convert local position to ECEF relative
      coordinates */
  Matrix to_ECEF;

  /** Vector in ECEF to the lat-lon-zero origin */
  ECEF origin;

  /** Local Radius meridian */
  double RM;

    /** Local Radius parallel */
  double RP;

public:
  /** Constructor */
  LocalAxis(const LatLonAlt &lla, double psi_zero = 0.0);

  /** Create an ECEF representation from a local xy_altitude set */
  ECEF toECEF(const Carthesian &coords) const;

  /** Create a local representation from an ECEF location */
  Carthesian toLocal(const ECEF &ecef) const;

  /** Orientation conversion */
  Orientation toNorthUp(const Orientation &o) const;
};

/** Base class for flightgear axes. Derived class implements ecef
    coordinates, or coordinates with respect to a local axis somewhere */
struct FGAxis
{
  /** Constructor */
  FGAxis();

  /** Destructor */
  virtual ~FGAxis();

    /** Result of this transformation is latitude, in degrees,
      longitude, in degrees, and geodetic altitude, in feet,
      next roll, pitch and yaw angles, also in degrees.
      \param  result vector for result
      \param  xyz    position vector
      \param  quat   attitude quaternion. */
  virtual void transform(double result[6], const double xyz[3],
                         const double quat[4]) = 0;

  /** Result of this transformation is ECEF position, double, in m
      Standard ECEF, x through Greenwich meridian, y through east, z upwards */
  virtual void toECEF(double pos[3], float velocity[3], float attitude[4],
                      float omega[3], const double xyz[3], const double quat[4],
                      const float uvw[3], const float pqr[3]) const = 0;
};

/** Axis transformation from a local reference frame to FlightGear
    coordinates.

    The local reference frame is with x-axis horizontal and pointing
    to a heading \f$ \Psi_0 \f$, and a y axis horizontal and pointing
    perpendicular to the x-axis, and the z-axis pointing down.

    The FlightGear coordinates are lat-lon-alt for the own vehicle,
    and an ECEF framework (x pointing through the GW or prime meridian, y
    pointing 90 degrees east, and z pointing "up", through the north
    pole.
*/
struct FGLocalAxis : public FGAxis
{
  /** Heading offset local axis */
  double psi_zero;

  /** Local axis itself */
  LocalAxis axis;

  /** Orientation correction to north up */
  Orientation q_psi0;

  /** Constructor. Assumes a world coordinate system with
      <ol>
      <li> x pointing psi_zero, y pointing psi_zero+90, z pointing down
      </ol>
      \param lat_zero Geodetic latitude of origin
      \param lon_zero Longitude of origin
      \param h_zero   Altitude of origin, m
      \param psi_zero Local axis direction */
  FGLocalAxis(double lat_zero, double lon_zero, double h_zero, double psi_zero);

  /** Result of this transformation is latitude, in degrees,
      longitude, in degrees, and geodetic altitude, in feet,
      next roll, pitch and yaw angles, also in degrees.
      \param  result vector for result
      \param  xyz    position vector
      \param  quat   attitude quaternion. */
  void transform(double result[6], const double xyz[3], const double quat[4]);

  /** Result of this transformation is ECEF position, double, in m
      Standard ECEF, x through Greenwich meridian, y through east, z upwards */
  void toECEF(double pos[3], float velocity[3], float attitude[4],
              float omega[3], const double xyz[3], const double quat[4],
              const float uvw[3], const float pqr[3]) const;
};

/** Axis transformation from ECEF to FlightGear coordinates.
    X points through greenwich medidian,
    Z points to south pole
    Y points due west. */
struct FGECEFAxis : public FGAxis
{
  /** Constructor. */
  FGECEFAxis();

    /** Result of this transformation is latitude, in degrees,
      longitude, in degrees, and geodetic altitude, in feet,
      next roll, pitch and yaw angles, also in degrees.
      \param  result vector for result
      \param  xyz    position vector, wrt ecef
      \param  quat   attitude quaternion, wrt ecef */
  void transform(double result[6], const double xyz[3], const double quat[4]);

  /** Result of this transformation is ECEF position, double, in m
      Standard ECEF, x through Greenwich meridian, y through east, z upwards */
  void toECEF(double pos[3], float velocity[3], float attitude[4],
              float omega[3], const double xyz[3], const double quat[4],
              const float uvw[3], const float pqr[3]) const;
};

#endif
