  /** Position and attitude are linearly extrapolated. */
  void extrapolate(double dt);
  
  /** Accept quaternion from phi, theta, psi */
  void setquat(double phi, double tht, double psi);

  /** get roll angle, returns result in radians */
  inline double getPhi() const
{ return atan2(2.0*double(attitude_q[2]*attitude_q[3] + 
			  attitude_q[1]*attitude_q[0]), 
    double(attitude_q[0]*attitude_q[0] - 
	   attitude_q[1]*attitude_q[1] - 
	   attitude_q[2]*attitude_q[2] + 
	   attitude_q[3]*attitude_q[3])); }

  /** get pitch angle, returns result in radians */
  inline double getTht() const
  { return asin(-2.0*double(attitude_q[1]*attitude_q[3] -
		            attitude_q[2]*attitude_q[0])); }

  /** get yaw angle, returns result in radians */
  inline double getPsi() const
  { return atan2(2.0*double(attitude_q[1]*attitude_q[2] + 
			    attitude_q[3]*attitude_q[0]), 
      double(attitude_q[0]*attitude_q[0] + 
		 attitude_q[1]*attitude_q[1] - 
		 attitude_q[2]*attitude_q[2] - 
	     attitude_q[3]*attitude_q[3])); }

  /** translate the given objectmotion object into an object relative
      to the current one. */
  ObjectMotion getRelative(const ObjectMotion& o) const;
