/* ------------------------------------------------------------------   */
/*      item            : WorldDataSpecExtra.hxx
        made by         : Rene van Paassen
        date            : 170327
	category        : header file addition
        description     :
	changes         : 170327 converted from hand-coded
        language        : C++
*/

/** Check that this is sufficiently specified */
bool sufficient();

/** Add/replace a number of coordinates */
template<typename T>
void setCoordinates(unsigned offset, unsigned ncoord, const T& values)
{
  if (ncoord + offset > coordinates.size()) {
    coordinates.resize(offset + ncoord);
  }
  for (unsigned idx = 0; idx < ncoord && idx < values.size(); idx++) {
    coordinates[offset+idx] = values[idx];
  }
}

/** Constructor that takes an partial WorldDataSpec */
WorldDataSpec(const WorldDataSpec& o, unsigned nfstrip, unsigned ncstrip=0U);
