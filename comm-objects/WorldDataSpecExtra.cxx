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
