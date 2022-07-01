  /** Convert from an ObjectMotion object, using a localaxis object */
  void setCommand(const ObjectMotion& obj, const FGLocalAxis& ax);

  /** Convert from x, y , z, phi, theta, psi */
  void setCommand(const double xyz[3], 
		  const double phi, const double tht, const double psi,
		  const FGLocalAxis& ax);

  /** Convert from a position vector and a quaternion */
  void setCommand(const double xyz[3], const double quat[4],
		  const FGLocalAxis& ax);
