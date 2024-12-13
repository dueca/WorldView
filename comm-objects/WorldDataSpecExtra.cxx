/* ------------------------------------------------------------------   */
/*      item            : WorldDataSpecExtra.cxx
        made by         : Rene van Paassen
        date            : 170327
	category        : body file addition
        description     :
	changes         : 170327 converted from hand-coded
        language        : C++
*/

/** Check that this is sufficiently specified */
bool WorldDataSpec::sufficient()
{
  return name.size() != 0 && type.size() != 0;
}

WorldDataSpec::WorldDataSpec(const WorldDataSpec& o,
                             unsigned nfstrip, unsigned ncstrip) :
  name(o.name),
  parent(o.parent),
  type(o.type)
{
  if (o.filename.size() > nfstrip) {
    filename.resize(o.filename.size() > nfstrip);
    for (unsigned i = 0U; i < o.filename.size() - nfstrip; i++) {
      filename[i] = o.filename[i];
    }
  }
  if (o.coordinates.size() > nfstrip) {
    coordinates.resize(o.coordinates.size() - ncstrip);
    for (unsigned i = 0U; i < o.filename.size() - nfstrip; i++) {
      coordinates[i] = o.coordinates[i];
    }
  }
}
