/* ------------------------------------------------------------------   */
/*      item            : ViewSpecExtra.cxx
        made by         : Rene van Paassen
        date            : 170327
	category        : body file addition
        description     :
	changes         : 170327 converted from hand-coded
        language        : C++
*/

/** Check that this is sufficiently specified */
bool ViewSpec::sufficient()
{
  return ( name.size() != 0 ) &&
    ( portcoords.size() == 0 || portcoords.size() == 4 ) &&
    ( frustum_data.size() == 3 || frustum_data.size() == 6 ) &&
    ( shadow_colour.size() == 3 );
}
