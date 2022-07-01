/* ------------------------------------------------------------------   */
/*      item            : GLRateInfo.cxx
        made by         : Rene' van Paassen
        date            : 100209
	category        : body file 
        description     : Rate information on your display. Uses the 
	                  fontmanager from hmilib, if you don't have 
			  hmilib listed in your main makefile, you
                          won't have this plib object.
	changes         : 100209 first version
        language        : C++
*/


#ifndef DUECA_CONFIG_HMI
#warning "hmilib not detected, not building displayrate object"
#else

#define GLRateInfo_cxx
#include "GLRateInfo.hxx"
#include "PlibObjectFactory.hxx"
#include <DataPair.hxx>
#include <FontManager.hxx>
#include <GL/glu.h>
#include <iterator>

using namespace hmi;
using namespace std;

GLRateInfo::GLRateInfo(const WorldDataSpec& spec) :
  PlibObject(spec),
  rate(0),
  gap(0),
  currentrate(0),
  currentgap(0)
{
  previous.tv_sec = 0;
  previous.tv_usec = 0;
  insert_iterator<list<int> > ii(drawviews, drawviews.begin());
  copy(spec.coordinates.begin(), spec.coordinates.end(), ii);
}


GLRateInfo::~GLRateInfo()
{
  //
}

void GLRateInfo::draw(int viewno)
{ 
  if (find(drawviews.begin(), drawviews.end(), viewno) == drawviews.end()) 
    return;

  // measure time now
  struct timeval tvnow;
  gettimeofday(&tvnow, NULL);

  // calculate how many microseconds difference between current and
  // previous time
  int usec_diff = int(tvnow.tv_usec) - int(previous.tv_usec);
  if (tvnow.tv_sec > previous.tv_sec) {
    // copy the statistics over the previous second
    gap = currentgap; rate = currentrate;

    // re-counting again
    currentrate = 0;
    currentgap = 0;
    
    // correct for the second step with the microsecond gap
    usec_diff += 1000000;
  }
  if (usec_diff > currentgap) currentgap = usec_diff;
  currentrate++;

  // remember current
  previous = tvnow;

  FontManager::Init();

  // I am breaking in on the PLIB GL work here. Be very careful, reset
  // everything after doing this draw routine
  glMatrixMode(GL_PROJECTION);
  glPushMatrix(); // saves current (PLIB's)

  // define new 2d Projection matrix
  glLoadIdentity();
  gluOrtho2D(0.0, 500.0, 0.0, 500.0);
  
  // define identity model matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix(); // saves current (PLIB's)
  glLoadIdentity();  

  // save attributes, a bit coarse, might save space and time by
  // selecting exactly which attributes I change here!
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  
  // turn alpha blending on
  glDisable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  // draw a semi-transparant (alpha blended) box
  // for the message     
  glColor4f(0.5,0.5,0.5,0.7); // 0.6 alpha
  glBegin(GL_QUADS);
      glVertex2f( 10.0, 10.0);
      glVertex2f(300.0, 10.0);
      glVertex2f(300.0, 50.0);
      glVertex2f( 10.0, 50.0);
  glEnd();

  // alpha blending off
  glDisable(GL_BLEND);

  Font *font = FontManager::Get(1);
  font->SetSize(0.1, 0.1);

  // draw the text message inside the box. Don't have, don't like glut
  glColor3f(1.0,1.0,1.0); // white
  char buf[100];
  snprintf(buf, sizeof(buf), "rate %3i  **  longest gap %6i us", rate, gap);
  font->Print(20.0, 25.0, buf);
  
  // restore the attributes
  glPopAttrib();

  // restore the projection and model matrices
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
};
  
// By creating this subcontractor, the factory for creating graphics
// objects used by PlibViewer is extended with the possibility to
// create displayrate objects. 
static SubContractor<PlibObjectTypeKey, GLRateInfo>
*GLRateInfo_maker = new 
  SubContractor<PlibObjectTypeKey, GLRateInfo>("displayrate");

#endif
