  /** Position and attitude are linearly extrapolated. */
  void extrapolate(double dt);
  
  /** translate the given objectmotion object into an object relative
      to the current one. */
  BaseObjectMotion getRelative(const BaseObjectMotion& o) const;
