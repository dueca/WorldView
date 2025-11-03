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
template <typename T>
void setCoordinates(unsigned offset, unsigned ncoord, const T &values)
{
  if (ncoord + offset > coordinates.size()) {
    coordinates.resize(offset + ncoord);
  }
  for (unsigned idx = 0; idx < ncoord && idx < values.size(); idx++) {
    coordinates[offset + idx] = values[idx];
  }
}

/** Add/replace a number of file strings */
template <typename T>
void setStrings(unsigned offset, unsigned ns, const T &values)
{
  if (ncoord + offset > filename.size()) {
    filename.resize(offset + ns);
  }
  for (unsigned idx = 0; idx < ns && idx < values.size(); idx++) {
    filename[offset + idx] = values[idx];
  }
}

/** Constructor that takes an partial WorldDataSpec */
WorldDataSpec(const WorldDataSpec &o, unsigned nfstrip, unsigned ncstrip = 0U);
