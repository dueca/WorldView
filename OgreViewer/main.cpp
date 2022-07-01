/* ------------------------------------------------------------------   */
/*      item            : main.cxx
        made by         : Rene' van Paassen
        date            : 090620
	category        : body file 
        description     : 
	changes         : 090620 first version
        language        : C++
*/

#define main_cxx
#include "OgreViewer.hxx"
#include "OgreObjectBase.hxx"
#include <unistd.h>

#define TWIN

using namespace std;

int main(int argc, char* argv[]) 
{
  // make a viewer
  OgreViewer *viewer = new OgreViewer("test view", true);
  
  // define 3 cameras, looking in different directions
  ViewSpec cam;
  {
    cam.name = "left";
#ifdef TWIN
    cam.winname = "1";
#endif
    cam.overlay = "HMILabOverlays/MaskLeftSide";
    float pos_and_rot[] = {  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f };
    copy(&pos_and_rot[0], &pos_and_rot[6], cam.eye_pos.begin());
    int port[] = { 0, 0, 400, 300 };
    cam.portcoords.resize(4); 
    copy(&port[0], &port[4], cam.portcoords.begin());
    float frust[] = { 1.0f, 1000.0f, -1.0f, 1.0f, 0.75f, -0.75f };  
    cam.frustum_data.resize(6); 
    copy(&frust[0], &frust[6], cam.frustum_data.begin());
  }
  viewer->addViewport(cam);
  
  {
    cam.name = "center";
#ifdef TWIN
    cam.winname = "1";
#endif
    cam.overlay = "HMILabOverlays/MaskLeftFront";
    float pos_and_rot[] = {  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    copy(&pos_and_rot[0], &pos_and_rot[6], cam.eye_pos.begin());
    int port[] = { 400, 0, 400, 300 };
    cam.portcoords.resize(4);
    copy(&port[0], &port[4], cam.portcoords.begin());
    float frust[] = { 1.0f, 1000.0f, -1.0f, 1.0f, 0.75f, -0.75f };  
    cam.frustum_data.resize(6); 
    copy(&frust[0], &frust[6], cam.frustum_data.begin());
  }
  viewer->addViewport(cam);
   
  {
    cam.name = "right";
#ifdef TWIN
    cam.winname = "2";
#endif
    cam.overlay = "HMILabOverlays/MaskRightSide";
    float pos_and_rot[] = {  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f };
    copy(&pos_and_rot[0], &pos_and_rot[6], cam.eye_pos.begin());
#ifdef TWIN
    int port[] = { 0, 0, 1024, 768 };
#else
    int port[] = { 800, 0, 400, 300 };
#endif
    cam.portcoords.resize(4);
    copy(&port[0], &port[4], cam.portcoords.begin());
    float frust[] = { 1.0f, 1000.0f, -1.0f, 1.0f, 0.75f, -0.75f };  
    cam.frustum_data.resize(6);
    copy(&frust[0], &frust[6], cam.frustum_data.begin());
  }
  viewer->addViewport(cam);
  
#ifdef TWIN
  {
    WinSpec window;
    window.name = "1";
    float size_and_position[] = { 800, 300, 0, 0};
    window.size_and_position.resize(4);
    copy(&size_and_position[0], &size_and_position[4],
	 window.size_and_position.begin());
    window.display = ":0.0";
    viewer->addWindow(window);
  }
  {
    WinSpec window;
    window.name = "2";
    float size_and_position[] = { 1024, 768};
    window.size_and_position.resize(2);
    copy(&size_and_position[0], &size_and_position[2],
	 window.size_and_position.begin());
    window.display = ":0.0";
    viewer->addWindow(window);
  }
#endif
      

  viewer->addScene("sample.scene");
  //viewer->addScene("sample2.scene");
  // int
  viewer->init();

  // run a number of cycles
  double psi = 0.0;
  for (int ii = 1000; ii--; ) {
    viewer->redraw();
    psi += 0.5;
    viewer->setBase(0, 0, 0, 0.0, 0, psi);
    usleep(100000);
  }
}
  
