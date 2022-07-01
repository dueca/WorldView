/* ------------------------------------------------------------------   */
/*      item            : WinSpecExtra.cxx
        made by         : Rene van Paassen
        date            : 170327
	category        : body file addition
        description     :
	changes         : 170327 converted from hand-coded
        language        : C++
*/

/** Check that this is sufficiently specified */
bool WinSpec::sufficient()
{
  return name.size() != 0 &&
    ( size_and_position.size() == 2 || size_and_position.size() == 4 );
}
