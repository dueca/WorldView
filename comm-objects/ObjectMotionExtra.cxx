#include <cmath>
#include <integrate_rungekutta.hxx>
#define __CUSTOM_COMPATLEVEL_110

#ifdef USING_EIGEN3
typedef const Eigen::Map<const Eigen::VectorXd> cVectorE;
typedef Eigen::Map<Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> > MatrixE;
#else
#include <mtl/mtl.h>
typedef mtl::matrix<double, mtl::rectangle<>, 
                    mtl::dense<mtl::external>, mtl::row_major>::type MatrixE;
typedef mtl::external_vec<double> VectorE;
typedef const mtl::external_vec<const double> cVectorE;
#endif

static const int qX = 1;
static const int qY = 2;
static const int qZ = 3;
static const int qW = 0;

/** Helper class for the integration/extrapolation of body motion with 
    constant speed along the body axes and a constant rotation, also
    in body axes. */
class ObjectMotion_Body 
{
  /** State, combines the attitude and the position */
  VectorE x;
  /** Speed of the body, in body axis system */
  VectorE uvw;
  /** Rotational speed of the body, in body axis system. */
  VectorE omega;

public:
  /** Constructor. Note that d_x is taken as a 7 element
      vector. while in the ObjectMotion object it is composed of a 4
      element quaternion and a 3 element position.
      \param d_x   state, first quaternion, then position 
      \param d_uvw speedi in body axes, 3 elt.
      \param d_omega rotational speed in body axes, 3 elt. */
  ObjectMotion_Body(double *d_x, double* d_uvw, double *d_omega) :
    x(d_x, 7), uvw(d_uvw, 3), omega(d_omega, 3) { }

  /** Calculation of the derivative of the state, for time dt */
  inline void derivative(VectorE& xd, double dt) 
  {
    // first normalize the quaternion
#ifdef USING_EIGEN3
    x.head(4).normalize();
#else
    scale(x(0,4), 1.0/two_norm(x(0,4)));
#endif

    // as per Stevens + Lewis, DCM, is transverse, inertial->body
    double Uq_data[9];
    MatrixE Uq(Uq_data, 3, 3);
    Uq(0,0) = x[qW]*x[qW] + x[qX]*x[qX] - x[qY]*x[qY] - x[qZ]*x[qZ];
    Uq(0,1) = 2*(x[qX]*x[qY] + x[qZ]*x[qW]);
    Uq(0,2) = 2*(x[qX]*x[qZ] - x[qY]*x[qW]);
    Uq(1,0) = 2*(x[qX]*x[qY] - x[qZ]*x[qW]);
    Uq(1,1) = x[qW]*x[qW] - x[qX]*x[qX] + x[qY]*x[qY] - x[qZ]*x[qZ];
    Uq(1,2) = 2*(x[qY]*x[qZ] + x[qX]*x[qW]);
    Uq(2,0) = 2*(x[qX]*x[qZ] + x[qY]*x[qW]);
    Uq(2,1) = 2*(x[qY]*x[qZ] - x[qX]*x[qW]);
    Uq(2,2) = x[qW]*x[qW] - x[qX]*x[qX] - x[qY]*x[qY] + x[qZ]*x[qZ];

    // calculate derivative of xyz with the attitude, and add to
    // current pos
#ifdef USING_EIGEN3
    xd.segment(4,3) = Uq.transpose() * uvw;
#else
    mtl::set(xd(4,7), 0.0); mult(mtl::trans(Uq), uvw, xd(4,7));
#endif    
    xd[qW] = -0.5 * (omega[0]*x[qX] + omega[1]*x[qY] + omega[2]*x[qZ]);
    xd[qX] =  0.5 * (x[qW]*omega[0] - (omega[1]*x[qZ] - omega[2]*x[qY]));
    xd[qY] =  0.5 * (x[qW]*omega[1] - (omega[2]*x[qX] - omega[0]*x[qZ]));
    xd[qZ] =  0.5 * (x[qW]*omega[2] - (omega[0]*x[qY] - omega[1]*x[qX]));
  };

  /** Access the state vector */
  inline VectorE& X() {return x;}
  
  /** Modify the current state vector */
  inline void setState(const VectorE newx) 
  { 
#ifdef USING_EIGEN3
    x = newx;
#else
    mtl::copy(newx, x); 
#endif
  }

};

void ObjectMotion::extrapolate(double span)
{
  // no extrapolation
  if (dt == 0.0 || span <= 0.0) return;

  // use an ObjectMotion_Body object to calculate the derivative, 
  ObjectMotion_Body extrap(attitude_q, uvw, omega);
  if (dt < 0.0) {
    double xd_data[7]; VectorE xd(xd_data, 7);
    extrap.derivative(xd, span);
#ifdef USING_EIGEN3
    extrap.X() += xd * span;
#else
    add(scaled(xd, span), extrap.X());
#endif
  }
  else {
    int iterations = max(1, int(span / dt + 0.5));
    double step = span / iterations;
    double ws_data[6*7];
    RungeKuttaWorkspace workspace(ws_data, 7);
    for ( ; iterations--; ) {
      // run the integration
      integrate_rungekutta(extrap, workspace, step);
    }
  }
  
  // normalize the quaternion after this
#ifdef USING_EIGEN3
  VectorE q(attitude_q, 4); q.normalize();
#else
  VectorE q(attitude_q, 4); scale(q, 1.0/two_norm(q));
#endif
}
  
void ObjectMotion::setquat(double phi, double tht, double psi)
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


template<typename M, typename T>
inline void Q2R(M& mat, const T& q)
{
  double xx2 = 2 * q[qX] * q[qX];
  double yy2 = 2 * q[qY] * q[qY];
  double zz2 = 2 * q[qZ] * q[qZ];
  double xy2 = 2 * q[qX] * q[qY];
  double wz2 = 2 * q[qW] * q[qZ];
  double zx2 = 2 * q[qZ] * q[qX];
  double wy2 = 2 * q[qW] * q[qY];
  double yz2 = 2 * q[qY] * q[qZ];
  double wx2 = 2 * q[qW] * q[qX];
  mat(0,0) = 1. - yy2 - zz2; mat(0,1) = xy2 - wz2; mat(0,2) = zx2 + wy2;
  mat(1,0) = xy2 + wz2; mat(1,1) = 1. - xx2 - zz2; mat(1,2) = yz2 - wx2;
  mat(2,0) = zx2 - wy2; mat(2,1) = yz2 + wx2; mat(2,2) = 1. - xx2 - yy2;
}


template<class Mat, class Vec>
inline void cross_prod_matrix(Mat& m, const Vec& v)
{
  m(0,0) =   0.0; m(0,1) = -v[2]; m(0,2) =  v[1];
  m(1,0) =  v[2]; m(1,1) =   0.0; m(1,2) = -v[0];
  m(2,0) = -v[1]; m(2,1) =  v[0]; m(2,2) =   0.0;
}

template<class T, class U, class V>
inline void QxQ(T& result, const U& q1, const V& q2)
{
  result[qW] = q1[qW]*q2[qW] - q1[qX]*q2[qX] - q1[qY]*q2[qY] - q1[qZ]*q2[qZ];
  result[qX] = q1[qW]*q2[qX] + q1[qX]*q2[qW] + q1[qY]*q2[qZ] - q1[qZ]*q2[qY];
  result[qY] = q1[qW]*q2[qY] + q1[qY]*q2[qW] + q1[qZ]*q2[qX] - q1[qX]*q2[qZ];
  result[qZ] = q1[qW]*q2[qZ] + q1[qZ]*q2[qW] + q1[qX]*q2[qY] - q1[qY]*q2[qX];
}  

ObjectMotion ObjectMotion::getRelative(const ObjectMotion& o) const
{
  ObjectMotion result;

  // create a rotation matrix and a cross-product matrix
  double R1_data[9]; MatrixE R1(R1_data, 3, 3);
  double R2_data[9]; MatrixE R2(R2_data, 3, 3);
  Q2R(R1, this->attitude_q);
  Q2R(R2, o.attitude_q);
  double X_data[9]; MatrixE X(X_data, 3, 3);
  cross_prod_matrix(X, this->omega);

#ifdef USING_EIGEN3
  { 
    // convert position
    VectorE r_xyz(result.xyz, 3);
    r_xyz = R1.transpose()*( cVectorE(o.xyz, 3) - cVectorE(this->xyz, 3)); 

    // convert velocity
    VectorE r_uvw(result.uvw, 3);
    
    // R2^T ( R1 ( X (-xyz_r) ) + R2 uvw2 - R1 uvw1))
    // ^ transform uvw in world to R2 body
    //        ^ transform uvw in R1 to world
    //            ^body rot matrix * relative posn O2 in O1 coordinate
    //                          ^ transform uvw 2 to world
    //                                    ^ transform uvw 1 to world
    // = uvw2 - R2^T R1 (X xyz_r + uvw1) 
    r_uvw = cVectorE(o.uvw, 3) - R2.transpose() *
      ( R1 * ( X * r_xyz + cVectorE(this->uvw, 3)) );
  }
#else
  // convert position
  { 
    double data[3]; VectorE xyz(data, 3);
    // step 1, vector from my origin to the other object, world coordinates
    mtl::add(cVectorE(o.xyz, 3), scaled(cVectorE(this->xyz, 3), -1.0), xyz);
    // step 2, convert to my axes
    mtl::mult(trans(R1), xyz, VectorE(result.xyz, 3));
  }

  // convert velocity
  {
    double data1[3]; VectorE uvw1(data1, 3); 
    double data2[3]; VectorE uvw2(data2, 3);
    // step 1, my relative velocity vector in world coordinates
    mtl::mult(R1, cVectorE(this->uvw, 3), uvw1);
    // step 2, other relative velocity vector in world coordinates
    mtl::mult(R2, cVectorE(o.uvw, 3), uvw2);
    // step 3, subtract
    mtl::add(scaled(uvw1, -1.0), uvw2, uvw2);

    // step 4, get cross product of rot velocity, in B1 coordinates
    double ux_data[3]; VectorE ux(ux_data, 3);
    mtl::mult(X, scaled(cVectorE(result.xyz, 3), -1.0), ux);
    // setp 4b, convert that to world coordinates, + add to result
    mtl::mult(R1, ux, uvw2, uvw2);

    // now convert to B2 coordinates
    mtl::mult(trans(R2), uvw2, VectorE(result.uvw, 3));
  }

  // convert rotation
  QxQ(result.attitude_q, o.attitude_q, 
      scaled(cVectorE(this->attitude_q, 4), -1.0));

  // convert rotational speed,
  {
    double data1[3]; VectorE om1(data1, 3); 
    double data2[3]; VectorE om2(data2, 3);
    // step 1, my rotational speed vector in world coordinates
    mtl::mult(R1, cVectorE(this->omega, 3), om1);
    // step 2, other rotational speed vector in world coordinates
    mtl::mult(R2, cVectorE(o.omega, 3), om2);
    // step 3, subtract
    mtl::add(scaled(om1, -1.0), om2, om2);
    
    // now convert to B2 coordinates
    mtl::mult(trans(R2), om2, VectorE(result.omega, 3));
  }
#endif

  // acceleration still needs to be done

  return result;
}
