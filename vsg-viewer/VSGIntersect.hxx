/* ------------------------------------------------------------------   */
/*      item            : VSGIntersectModel.hxx
        made by         : Rene van Paassen
        date            : 230126
        category        : header file
        description     :
        changes         : 230126 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once

#include "VSGModel.hxx"
#include "VSGObjectFactory.hxx"
#include <vsg/utils/LineSegmentIntersector.h>

namespace vsgviewer {

  class VSGIntersect
  {
    /** One or more intersectors for this model, e.g., on the wheels,
        bumpers */
    std::vector<vsg::ref_ptr<vsg::LineSegmentIntersector> > intersectors;

    /** Transform by which the root is reverse-shifted to get intersector
        locality */
    vsg::ref_ptr<vsg::AbsoluteTransform> rev_transform;

    /** Channel token for sending intersector data */
    boost::scoped_ptr<ChannelWriteToken>       w_intersects;

  public:
    /** Constructor.

        @param data
        * data.name            - model name / write label
        * data.parent          - unused
        * data.filename[0]     - xml file with intersectors definition
        * data.filename[1]     - channel for intersector results
    */
    VSGIntersect(const WorldDataSpec& data);

    /** Destructor */
    ~VSGIntersect();

    /** Initialise the model with the VSG scene */
    void init(const vsg::ref_ptr<vsg::Group>& root,
              VSGViewer* master);

    /** Connect to the DUECA system */
    void connect(const GlobalId& master_id, const NameSet& cname,
                 entryid_type entry_id,
                 Channel::EntryTimeAspect time_aspect);
  };

}; // namespace
