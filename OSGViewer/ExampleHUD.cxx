/* ------------------------------------------------------------------   */
/*      item            : ExampleHUD.cxx
        made by         : Rene' van Paassen
        date            : 190430
	category        : body file 
        description     : 
	changes         : 190430 first version
        language        : C++
	copyright       : (c) 2019 TUDelft-AE-C&S
*/

#define ExampleHUD_cxx
#include "ExampleHUD.hxx"
#include <sstream>
#include <dueca/debug.h>

#include <rapidjson/document.h>

// see also
// https://github.com/openscenegraph/OpenSceneGraph/blob/master/examples/osghud/osghud.cpp
typedef rapidjson::Document JDocument;
typedef rapidjson::Value JValue;

#if 0

ExampleHUD::ExampleHUD(const WorldDataSpec& spec) :
  WorldObjectBase(),
  nodemask(0xffffffff)
{
  // new way of using spec; fname* contain JSONs
  for (auto const& data: spec.filename) {
    JDocument doc; doc.Parse(data.c_str());
    for (JValue::ConstValueIterator it = doc.Begin();
         it != doc.End(); ++it) {
      if (!strcmp(it->name.GetString(), "window")) {
        wname = it->value.GetString();
      } else if (!strcmp(it->name.GetString(), "window")) {
        vname = it->value.GetString();
      }
      else {
        W_MOD("ExampleHUD, unknown data key " << it->name.GetString());
      }
    }
  }
}

ExampleHUD::~ExampleHUD()
{
  //
}

void ExampleHUD::connect(const GlobalId& master_id, const NameSet& cname,
                         entryid_type entry_id,
                         Channel::EntryTimeAspect time_aspect)
{
  // connect the read token
  r_hud.reset(new ChannelReadToken
              (master_id, cname, ExampleHUDInfo::classname, entry_id,
               time_aspect, Channel::OneOrMoreEntries,
               Channel::JumpToMatchTime));
}

void ExampleHUD::init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master)
{
  camera = new osg::Camera;

  // get the master camera for this window, either first, or as given by
  osg::ref_ptr<osg::Camera> mastercam = master->getMainCamera(wname, vname);
  
  // set the projection matrix (adjust?)
  camera->setProjectionMatrix(osg::Matrix::ortho2D(-500, 500, -500, 500));

  // set the view matrix
  camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  camera->setViewMatrix(osg::Matrix::identity());
  
  // only clear the depth buffer
  camera->setClearMask(GL_DEPTH_BUFFER_BIT);
  
  // draw subgraph after main camera view.
  camera->setRenderOrder(osg::Camera::POST_RENDER);
  
  // we don't want the camera to grab event focus from the viewers
  // main camera(s).
  camera->setAllowEventFocus(false);

  // add a geode to the camera
  {
    osg::Geode* geode = new osg::Geode();
    std::string timesFont("fonts/arial.ttf");

    // turn lighting off for the text and disable depth test to
    // ensure it's always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::Vec3 position(-400.0f,-400.0f,0.0f);

    // some text
    {
      text = new  osgText::Text;
      geode->addDrawable( text );
      
      text->setFont(timesFont);
      text->setPosition(position);
      std::stringstream spd; spd << "speed" << 100;
      text->setText(spd.str().c_str());
    }

    
    // transform
    transform = new osg::PositionAttitudeTransform();
    geode->addChild(transform);

    // horizon line
    osg::Geometry* hline = new osg::Geometry;
    osg::Vec3Array *vertices = new osg::Vec3Array;
    float depth = -0.1;
    vertices->push_back(osg::Vec3(-300, 0, depth));
    vertices->push_back(osg::Vec3( 300, 0, depth));
    vertices->push_back(osg::Vec3(-200, 50, depth));
    vertices->push_back(osg::Vec3( 200, 50, depth));
    vertices->push_back(osg::Vec3(-200, -40, depth));
    vertices->push_back(osg::Vec3( 200, -40, depth));
    hline->setVertexArray(vertices);
    
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    hline->setNormalArray(normals, osg::Array::BIND_OVERALL);
    
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0,0.8f,0.2f));
    hline->setColorArray(colors, osg::Array::BIND_OVERALL);

    hline->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,3));
    
    osg::StateSet* ss = hline->getOrCreateStateSet();
    ss->setMode(GL_BLEND,osg::StateAttribute::ON);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    transform->addChild(geode);
  }

  camera->setGraphicsContext(mastercam->getGraphicsContext());
  camera->setViewport(mastercam->getViewport());
  
  camera->addChild(transform);

  //viewer->addSlave(camera, false);
}



void ExampleHUD::unInit(const osg::ref_ptr<osg::Group>& root)
{
  camera = NULL;
  transform = NULL;
  text = NULL;
}

void ExampleHUD::visible(bool vis)
{
  if (vis && transform->getNodeMask() == 0) {
    transform->setNodeMask(nodemask);
  }
  else if (!vis && transform->getNodeMask() != 0) {
    nodemask = transform->getNodeMask();
    transform->setNodeMask(0);
  }
}

void ExampleHUD::iterate(TimeTickType ts,
                         const BaseObjectMotion& base, double late)
{
  // read data from channel
  // update angles/transforms/text
}
#endif
