/* ------------------------------------------------------------------   */
/*      item            : PlibTestOverlay.cxx
        made by         : rvanpaassen
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Fri Mar 19 17:06:10 2010
	category        : body file 
        description     : 
	changes         : Fri Mar 19 17:06:10 2010 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
*/

static const char c_id[] =
"$Id: PlibTestOverlay.cxx,v 1.5 2019/03/04 16:54:39 rvanpaassen Exp $";

#define PlibTestOverlay_cxx
// include the definition of the helper class
#include "PlibTestOverlay.hxx"

// include the debug writing header, by default, write warning and 
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>
//#define DEBUG_GLSTATE

// include additional files needed for your calculation here
#ifdef DUECA_CONFIG_HMI
#include <DataPair.hxx>
#include <FontManager.hxx>
#include <GL/glu.h>

#define DO_INSTANTIATE
#include <VarProbe.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <CoreCreator.hxx>
#include <iomanip>

USING_DUECA_NS;
using namespace hmi;
using namespace std;

// Parameters to be inserted
const ParameterTable* PlibTestOverlay::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). */
    { "strings",
      new VarProbe<PlibTestOverlay, vector<string> >
      (&PlibTestOverlay::messages), 
      "enter one or more strings with messages you want to display" },
      
    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, "please give a description of this helper class"} };

  return parameter_table;
}

// constructor
PlibTestOverlay::PlibTestOverlay() :
  PlibObject(WorldDataSpec()),
  ScriptCreatable()
{
  
}

bool PlibTestOverlay::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  if (messages.size() == 0) {
    E_CNF("Need messages, please");
    return false;
  }
  current = messages.begin();

  return true;
}



// destructor
PlibTestOverlay::~PlibTestOverlay()
{
  //
}

void PlibTestOverlay::nextMessage()
{
  if (++current == messages.end()) {
    current = messages.begin();
  }
}

#ifdef DEBUG_GLSTATE
struct PlibTestOverlay_DebugGLState {
  GLenum cap;
  const char* description;
};

#define STATE_ENTRY(A) { (A) , #A }

static PlibTestOverlay_DebugGLState gl_states[] = {
  STATE_ENTRY(GL_ALPHA_TEST), 
  STATE_ENTRY(GL_AUTO_NORMAL), 
  STATE_ENTRY(GL_BLEND),  
#if GL_MAX_CLIP_PLANES >= 1
  STATE_ENTRY(GL_CLIP_PLANE0),  
#endif
#if GL_MAX_CLIP_PLANES >= 2
  STATE_ENTRY(GL_CLIP_PLANE1),  
#endif
#if GL_MAX_CLIP_PLANES >= 3
  STATE_ENTRY(GL_CLIP_PLANE2),  
#endif
#if GL_MAX_CLIP_PLANES >= 4
  STATE_ENTRY(GL_CLIP_PLANE3),  
#endif
#if GL_MAX_CLIP_PLANES >= 5
  STATE_ENTRY(GL_CLIP_PLANE4),  
#endif
#if GL_MAX_CLIP_PLANES >= 6
  STATE_ENTRY(GL_CLIP_PLANE5),  
#endif
  STATE_ENTRY(GL_COLOR_ARRAY),  
  STATE_ENTRY(GL_COLOR_LOGIC_OP),
  STATE_ENTRY(GL_COLOR_MATERIAL),
  STATE_ENTRY(GL_COLOR_SUM),
  STATE_ENTRY(GL_COLOR_TABLE),
  STATE_ENTRY(GL_CONVOLUTION_1D),
  STATE_ENTRY(GL_CONVOLUTION_2D),
  STATE_ENTRY(GL_CULL_FACE),
  STATE_ENTRY(GL_DEPTH_TEST),
  STATE_ENTRY(GL_DITHER),
  STATE_ENTRY(GL_EDGE_FLAG_ARRAY),
  STATE_ENTRY(GL_FOG),
  STATE_ENTRY(GL_FOG_COORD_ARRAY),
  STATE_ENTRY(GL_HISTOGRAM),
  STATE_ENTRY(GL_INDEX_ARRAY),
  STATE_ENTRY(GL_INDEX_LOGIC_OP),
#if GL_MAX_LIGHTS >= 1
  STATE_ENTRY(GL_LIGHT0),
#endif
#if GL_MAX_LIGHTS >= 2
   STATE_ENTRY(GL_LIGHT1),
#endif
#if GL_MAX_LIGHTS >= 3
   STATE_ENTRY(GL_LIGHT2),
#endif
#if GL_MAX_LIGHTS >= 4
   STATE_ENTRY(GL_LIGHT3),
#endif
#if GL_MAX_LIGHTS >= 5
   STATE_ENTRY(GL_LIGHT4),
#endif
#if GL_MAX_LIGHTS >= 6
   STATE_ENTRY(GL_LIGHT5),
#endif
#if GL_MAX_LIGHTS >= 7
   STATE_ENTRY(GL_LIGHT6),
#endif
#if GL_MAX_LIGHTS >= 8
   STATE_ENTRY(GL_LIGHT7),
#endif
  STATE_ENTRY(GL_LIGHTING),
  STATE_ENTRY(GL_LINE_SMOOTH),
  STATE_ENTRY(GL_LINE_STIPPLE),
  STATE_ENTRY(GL_MAP1_COLOR_4),
  STATE_ENTRY(GL_MAP1_INDEX),
  STATE_ENTRY(GL_MAP1_NORMAL),
  STATE_ENTRY(GL_MAP1_TEXTURE_COORD_1),
  STATE_ENTRY(GL_MAP1_TEXTURE_COORD_2),
  STATE_ENTRY(GL_MAP1_TEXTURE_COORD_3),
  STATE_ENTRY(GL_MAP1_TEXTURE_COORD_4),
  STATE_ENTRY(GL_MAP2_COLOR_4),
  STATE_ENTRY(GL_MAP2_INDEX),
  STATE_ENTRY(GL_MAP2_NORMAL),
  STATE_ENTRY(GL_MAP2_TEXTURE_COORD_1),
  STATE_ENTRY(GL_MAP2_TEXTURE_COORD_2),
  STATE_ENTRY(GL_MAP2_TEXTURE_COORD_3),
  STATE_ENTRY(GL_MAP2_TEXTURE_COORD_4),
  STATE_ENTRY(GL_MAP2_VERTEX_3),
  STATE_ENTRY(GL_MAP2_VERTEX_4),
  STATE_ENTRY(GL_MINMAX),
  STATE_ENTRY(GL_MULTISAMPLE),
  STATE_ENTRY(GL_NORMAL_ARRAY),
  STATE_ENTRY(GL_NORMALIZE),
  STATE_ENTRY(GL_POINT_SMOOTH),
  STATE_ENTRY(GL_POINT_SPRITE),
  STATE_ENTRY(GL_POLYGON_SMOOTH),
  STATE_ENTRY(GL_POLYGON_OFFSET_FILL),
  STATE_ENTRY(GL_POLYGON_OFFSET_LINE),
  STATE_ENTRY(GL_POLYGON_OFFSET_POINT),
  STATE_ENTRY(GL_POLYGON_STIPPLE),
  STATE_ENTRY(GL_POST_COLOR_MATRIX_COLOR_TABLE),
  STATE_ENTRY(GL_POST_CONVOLUTION_COLOR_TABLE),
  STATE_ENTRY(GL_RESCALE_NORMAL),
  STATE_ENTRY(GL_SAMPLE_ALPHA_TO_COVERAGE),
  STATE_ENTRY(GL_SAMPLE_ALPHA_TO_ONE),
  STATE_ENTRY(GL_SAMPLE_COVERAGE),
  STATE_ENTRY(GL_SCISSOR_TEST),
  STATE_ENTRY(GL_SECONDARY_COLOR_ARRAY),
  STATE_ENTRY(GL_SEPARABLE_2D),
  STATE_ENTRY(GL_STENCIL_TEST),
  STATE_ENTRY(GL_TEXTURE_1D),
  STATE_ENTRY(GL_TEXTURE_2D),
  STATE_ENTRY(GL_TEXTURE_3D),
  STATE_ENTRY(GL_TEXTURE_COORD_ARRAY),
  STATE_ENTRY(GL_TEXTURE_CUBE_MAP),
  STATE_ENTRY(GL_TEXTURE_GEN_Q),
  STATE_ENTRY(GL_TEXTURE_GEN_R),
  STATE_ENTRY(GL_TEXTURE_GEN_S),
  STATE_ENTRY(GL_TEXTURE_GEN_T),
  STATE_ENTRY(GL_VERTEX_ARRAY),
  STATE_ENTRY(GL_VERTEX_PROGRAM_POINT_SIZE),
  STATE_ENTRY(GL_VERTEX_PROGRAM_TWO_SIDE),
  {0, NULL}
};
#endif

void PlibTestOverlay::draw(int viewno)
{ 
  //cout << "testoverlay redraw \"" << *current << '"' << endl;

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

#ifdef DEBUG_GLSTATE
  static int testcount = 100;
  if (!testcount--) {
    for (PlibTestOverlay_DebugGLState* tt = gl_states; 
	 tt->description != NULL; tt++) {
      cout << std::setw(50) << tt->description << ": " 
	   << (glIsEnabled(tt->cap) == GL_TRUE ? "enabled" : "disabled") 
	   << endl;
    }
    testcount = 100;
  }
#endif
  
  // sane state for this work
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_FOG);
  glDisable(GL_COLOR_LOGIC_OP);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_COLOR_SUM);
  glDisable(GL_CULL_FACE);

  // turn alpha blending on
  //  glAlphaFunc(GL_ALWAYS, 0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  // draw a semi-transparant (alpha blended) box
  // for the message     
  glColor4f(0.5,0.5,0.5,0.7); // 0.6 alpha
  glBegin(GL_QUADS);
      glVertex2f( 10.0, 110.0);
      glVertex2f(300.0, 110.0);
      glVertex2f(300.0, 150.0);
      glVertex2f( 10.0, 150.0);
  glEnd();

  // alpha blending off
  glDisable(GL_BLEND);

  Font *font = FontManager::Get(1);
  font->SetSize(0.1, 0.1);

  // draw the text message inside the box. Don't have, don't like glut
  glColor4f(1.0,1.0,1.0, 1.0); // white
  if (current != messages.end()) 
    font->Print(20.0, 125.0, current->c_str());
  
  // restore the attributes
  glPopAttrib();

  // restore the projection and model matrices
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
};


// script access macro
SCM_FEATURES_NOIMPINH(PlibTestOverlay, ScriptCreatable, "plib-test-overlay");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type
static CoreCreator<PlibTestOverlay> a(PlibTestOverlay::getParameterTable());

#endif
