#include <cmath>
#include <integrate_rungekutta.hxx>
#define __CUSTOM_COMPATLEVEL_110


static const int qX = 1;
static const int qY = 2;
static const int qZ = 3;
static const int qW = 0;

/** Helper class for the integration/extrapolation of body motion with 
    constant speed along the body axes and a constant rotation, also
    in body axes. */
class BaseObjectMotion_Body 
{
public:
  typedef Eigen::Map<Eigen::VectorXd> VectorE;
  typedef const Eigen::Map<const Eigen::VectorXd> cVectorE;
  typedef Eigen::Map<Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> > MatrixE;
  typedef Eigen::Matrix<double,3,3,Eigen::RowMajor> Matrix33;
  typedef const Eigen::Map<const Eigen::VectorXf> cVectorEf;
  typedef Eigen::Map<Eigen::VectorXf> VectorEf;
private:
  
  /** State, combines the attitude and the position */
  VectorE x;
  /** Speed of the body, in body axis system */
  cVectorEf uvw;
  /** Rotational speed of the body, in body axis system. */
  cVectorEf omega;

public:
  /** Constructor. Note that d_x is taken as a 7 element
      vector. while in the ObjectMotion object it is composed of a 4
      element quaternion and then a 3 element position.
      \param d_x   state, first quaternion, then position 
      \param d_uvw speed in body axes, 3 elt, float.
      \param d_omega rotational speed in body axes, 3 elt, float. */
  BaseObjectMotion_Body(double *d_x, float* f_uvw, float *f_omega) :
    x(d_x, 7), uvw(f_uvw, 3), omega(f_omega, 3) { }

  /** Calculation of the derivative of the state, for time dt */
  inline void derivative(VectorE& xd, float dt) 
  {
    // first normalize the quaternion
    x.head(4).normalize();

    // as per Stevens + Lewis, DCM, is transverse, inertial->body
    Matrix33 Uq;
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
    xd.segment(4,3) = Uq.transpose() * uvw.cast<double>();

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
    x = newx;
  }

};

void BaseObjectMotion::extrapolate(double span)
{
  // no extrapolation
  if (dt == 0.0 || span <= 0.0) return;

  // use an ObjectMotion_Body object to calculate the derivative, 
  BaseObjectMotion_Body extrap(attitude_q, uvw, omega);
  if (dt < 0.0) {
    double xd_data[7]; BaseObjectMotion_Body::VectorE xd(xd_data, 7);
    extrap.derivative(xd, span);
    extrap.X() += xd * span;
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
  BaseObjectMotion_Body::VectorE q(attitude_q, 4); q.normalize();
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

BaseObjectMotion BaseObjectMotion::getRelative(const BaseObjectMotion& o) const
{
  BaseObjectMotion result;

  // create a rotation matrix and a cross-product matrix
  double R1_data[9]; BaseObjectMotion_Body::MatrixE R1(R1_data, 3, 3);
  double R2_data[9]; BaseObjectMotion_Body::MatrixE R2(R2_data, 3, 3);
  Q2R(R1, this->attitude_q);
  Q2R(R2, o.attitude_q);
  double X_data[9]; BaseObjectMotion_Body::MatrixE X(X_data, 3, 3);
  cross_prod_matrix(X, this->omega);

  { 
    // convert position
    BaseObjectMotion_Body::VectorE r_xyz(result.xyz, 3);
    r_xyz = R1.transpose()*( BaseObjectMotion_Body::cVectorE(o.xyz, 3) -
                             BaseObjectMotion_Body::cVectorE(this->xyz, 3)); 

    // convert velocity
    BaseObjectMotion_Body::VectorEf r_uvw(result.uvw, 3);
    
    // R2^T ( R1 ( X (-xyz_r) ) + R2 uvw2 - R1 uvw1))
    // ^ transform uvw in world to R2 body
    //        ^ transform uvw in R1 to world
    //            ^body rot matrix * relative posn O2 in O1 coordinate
    //                          ^ transform uvw 2 to world
    //                                    ^ transform uvw 1 to world
    // = uvw2 - R2^T R1 (X xyz_r + uvw1) 
    r_uvw = 
      BaseObjectMotion_Body::cVectorEf(o.uvw.ptr(), 3) -
      (R2.transpose() *
      ( R1 *
        ( X * r_xyz +
          BaseObjectMotion_Body::
          cVectorEf(this->uvw.ptr(), 3).cast<const double>() ) )
             ).cast<const float>();

    // convert rotation
    QxQ(result.attitude_q, o.attitude_q,
        - BaseObjectMotion_Body::cVectorE(this->attitude_q, 4));
  }

  BaseObjectMotion_Body::VectorEf omegar(result.omega, 3);
  omegar = (R2.transpose()*
    (R1*BaseObjectMotion_Body::cVectorEf(this->omega, 3).cast<const double>() -
     R2*BaseObjectMotion_Body::cVectorEf(o.omega, 3).cast<const double>())).cast<const float>();

  // acceleration still needs to be done
  return result;
}
