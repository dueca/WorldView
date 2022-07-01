  /** Accept quaternion from phi, theta, psi */
  void setquat(double phi, double tht, double psi);

  /** get roll angle, returns result in radians */
  double getPhi() const;

  /** get pitch angle, returns result in radians */
  double getTht() const;

  /** get yaw angle, returns result in radians */
  double getPsi() const;

  /** Fill a rotation matrix with the orientation */
  template <class M> 
  inline void orientationToR(M& mat) const
  {
    const int qX = 1;
    const int qY = 2;
    const int qZ = 3;
    const int qW = 0;
    mat(0,0) = -2.0 * (attitude_q[qY]*attitude_q[qY] +
		       attitude_q[qZ]*attitude_q[qZ]) + 1.0;
    mat(0,1) =  2.0 * (attitude_q[qX]*attitude_q[qY] -
		       attitude_q[qW]*attitude_q[qZ]);
    mat(0,2) =  2.0 * (attitude_q[qZ]*attitude_q[qX] +
		       attitude_q[qW]*attitude_q[qY]);
    mat(1,0) =  2.0 * (attitude_q[qX]*attitude_q[qY] +
		       attitude_q[qW]*attitude_q[qZ]);
    mat(1,1) = -2.0 * (attitude_q[qX]*attitude_q[qX] +
		       attitude_q[qZ]*attitude_q[qZ]) + 1.0;
    mat(1,2) =  2.0 * (attitude_q[qY]*attitude_q[qZ] -
		       attitude_q[qW]*attitude_q[qX]);
    mat(2,0) =  2.0 * (attitude_q[qZ]*attitude_q[qX] -
		       attitude_q[qW]*attitude_q[qY]);
    mat(2,1) =  2.0 * (attitude_q[qY]*attitude_q[qZ] +
		       attitude_q[qW]*attitude_q[qX]);
    mat(2,2) = -2.0 * (attitude_q[qX]*attitude_q[qX] +
		       attitude_q[qY]*attitude_q[qY]) + 1.0;
  }
