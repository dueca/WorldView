/* ------------------------------------------------------------------   */
/*      item            : GLOverlay.cxx
        made by         : Rene' van Paassen
        date            : 100210
	category        : body file 
        description     : 
	changes         : 100210 first version
        language        : C++
*/

#define GLOverlay_cxx
#include "GLOverlay.hxx"
#include "PlibObjectFactory.hxx"
#include <iterator>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <png.h>
#include <cstdio>
#include <string>
using namespace std;


 
#define TEXTURE_LOAD_ERROR 0
 
/** loadTexture
  	loads a png file into an opengl texture object, using 
	cstdio , libpng, and opengl.
	
	from en.wikibooks.org/wiki/OpenGL_Programming/Intermediate/Textures
 
  	\param filename : the png file to be loaded
  	\param width : width of png, to be updated as a side effect of 
	               this function
  	\param height : height of png, to be updated as a side effect 
                        of this function
  	\return GLuint : an opengl texture id.  Will be 0 if there is
  	                a major error,
	 		should be validated by the client of this
  	function.
*/
static GLuint loadTexture(const string filename, int &width, int &height) 
{
  //header for testing if it is a png
  png_byte header[8];
  
  //open file as binary
  FILE *fp = fopen(filename.c_str(), "rb");
  if (!fp) {
    return TEXTURE_LOAD_ERROR;
  }
  
  //read the header
  fread(header, 1, 8, fp);
  
  //test if png
  int is_png = !png_sig_cmp(header, 0, 8);
  if (!is_png) {
    fclose(fp);
    return TEXTURE_LOAD_ERROR;
  }
  
  //create png struct
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
					       NULL, NULL);
  if (!png_ptr) {
    fclose(fp);
    return (TEXTURE_LOAD_ERROR);
  }
 
  //create png info struct
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
    fclose(fp);
    return (TEXTURE_LOAD_ERROR);
  }
  
  //create png info struct
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    fclose(fp);
    return (TEXTURE_LOAD_ERROR);
  }
  
  //png error stuff, not sure libpng man suggests this.
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return (TEXTURE_LOAD_ERROR);
  }
  
  //init png reading
  png_init_io(png_ptr, fp);
  
  //let libpng know you already read the first 8 bytes
  png_set_sig_bytes(png_ptr, 8);
  
  // read all the info up to the image data
  png_read_info(png_ptr, info_ptr);
  
  //variables to pass to get info
  int bit_depth, color_type;
  png_uint_32 twidth, theight;
  
  // get info about png
  png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type,
	       NULL, NULL, NULL);
  
  //update width and height based on png info
  width = twidth;
  height = theight;
  
  // Update the png info struct.
  png_read_update_info(png_ptr, info_ptr);
  
  // Row size in bytes.
  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  
  // Allocate the image_data as a big block, to be given to opengl
  png_byte *image_data = new png_byte[rowbytes * height];
  if (!image_data) {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return TEXTURE_LOAD_ERROR;
  }
  
  //row_pointers is for pointing to image_data for reading the png with libpng
  png_bytep *row_pointers = new png_bytep[height];
  if (!row_pointers) {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    delete[] image_data;
    fclose(fp);
    return TEXTURE_LOAD_ERROR;
  }
  
  // set the individual row_pointers to point at the correct offsets
  // of image_data 
  for (int i = 0; i < height; ++i)
    row_pointers[height - 1 - i] = image_data + i * rowbytes;
  
  //read the png into image_data through row_pointers
  png_read_image(png_ptr, row_pointers);
  
  //Now generate the OpenGL texture object
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, width, height, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) image_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
  //clean up memory and close stuff
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  delete[] image_data;
  delete[] row_pointers;
  fclose(fp);
  
  return texture;
}

const static double def_vpoints[8] = 
  { 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0 };

GLOverlay::GLOverlay(const WorldDataSpec& spec) :
  PlibObject(spec),
  filename(),
  texture(0),
  vpoints(),
  viewno(0)
{
  if (spec.filename.size() && spec.filename[0].size()) {
    filename = spec.filename[0];
  }
  if (spec.coordinates.size() > 0) {
    viewno = spec.coordinates[0];
  }
  insert_iterator<vector<double> > ii(vpoints, vpoints.begin());
  if (spec.coordinates.size() == 9) {
    vector<double>::const_iterator jj = spec.coordinates.begin(); jj++;
    copy(jj, spec.coordinates.end(), ii);
  }
  else if (spec.coordinates.size() != 1) {
    cerr << "GLOverlay, " << spec.coordinates.size() 
         << " coordinates (need 8), using defaults" << endl;
  }
}

GLOverlay::~GLOverlay()
{
  
}

void GLOverlay::draw(int viewno)
{
  if (viewno != this->viewno || texture == TEXTURE_LOAD_ERROR) 
    return;

  // I am breaking in on the PLIB GL work here. Be very careful, reset
  // everything after doing this draw routine
  glMatrixMode(GL_PROJECTION);
  glPushMatrix(); // saves current (PLIB's)

  // define new 2d Projection matrix
  glLoadIdentity();
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport );
  gluOrtho2D(0.0, 1.0*viewport[2], 0.0, 1.0*viewport[3]);
  
  // define identity model matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix(); // saves current (PLIB's)
  glLoadIdentity();  

  // save attributes, a bit coarse, might save space and time by
  // selecting exactly which attributes I change here!
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  
  // set-up
  glDisable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_QUADS);
  glNormal3f(0.0, 0.0, 1.0);
  if (vpoints.size()) {
    glTexCoord2f(0.0f, 0.0f); glVertex2d(vpoints[0], vpoints[1]);
    glTexCoord2f(1.0f, 0.0f); glVertex2d(vpoints[2], vpoints[3]);
    glTexCoord2f(1.0f, 1.0f); glVertex2d(vpoints[4], vpoints[5]);
    glTexCoord2f(0.0f, 1.0f); glVertex2d(vpoints[6], vpoints[7]);
  }
  else {
    glTexCoord2f(0.0f, 0.0f);
    glVertex2d(0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2d(double(viewport[2]-viewport[0]), 0.0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2d(double(viewport[2]-viewport[0]),
               double(viewport[3]-viewport[1]));
    glTexCoord2f(0.0f, 1.0f);
    glVertex2d(0.0, double(viewport[3]-viewport[1]));
  }
  glEnd();
  
  // restore the attributes
  glPopAttrib();

  // restore the projection and model matrices
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

}

void GLOverlay::connect(const GlobalId& master_id, const NameSet& cname,
			entryid_type entry_id,
			Channel::EntryTimeAspect time_aspect)
{
  // the base overlay does not connect to a channel entry
}

void GLOverlay::iterate(TimeTickType ts,
			const BaseObjectMotion& base, double late)
{
  // ??
}
  
void GLOverlay::init(ssgRoot* scene) 
{
  int width, height;
  texture = loadTexture(filename, width, height);
}

static SubContractor<PlibObjectTypeKey, GLOverlay>
*GLOverlay_maker = new 
  SubContractor<PlibObjectTypeKey, GLOverlay>("overlay");
