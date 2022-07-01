#include <list>
#include <string>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
//#include <base/file_stream.hpp>
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>



using namespace std;
using namespace boost;

class italian_sandwich
{
    public:
  italian_sandwich() :
    m_bread("whole"),
    m_cheese("provo"),
    m_spicy_eggplant_p(false) { }

    private:
        string m_bread, m_cheese;
        list<string> m_meats;
        bool m_spicy_eggplant_p;

    friend class boost::serialization::access;
    template<class archive>
    void serialize(archive& ar, const unsigned int version)
    {
        using boost::serialization::make_nvp;
        ar & make_nvp("Bread", m_bread);
        ar & make_nvp("Cheese", m_cheese);
        ar & make_nvp("Meats", m_meats);
        ar & make_nvp("Add_Spicy_Eggplant", m_spicy_eggplant_p);
    }
};

void save_sandwich(const italian_sandwich& sw, const string& file_name)
{
  //typedef base::file_stream bafst;
  //  typedef str::ofstream bafst;
  // bafst::file_stream ofs(file_name, bafst::trunc | bafst::out);
  ofstream ofs; ofs.open(file_name.c_str() );
    boost::archive::xml_oarchive xml(ofs);
    xml << boost::serialization::make_nvp("Italian_Sandwich", sw);
}

void load_sandwich(italian_sandwich& sw, const string& file_name)
{
  //typedef base::file_stream bafst;
  //    italian_sandwich sw;
    //bafst::file_stream ifs(file_name, bafst::binary | bafst::in);
    ifstream ifs(file_name.c_str());
    boost::archive::xml_iarchive xml(ifs);
    xml >> boost::serialization::make_nvp("Italian_Sandwich", sw);
}

int main()
{
  italian_sandwich sw;
  
  save_sandwich(sw, "test.xml");
  load_sandwich(sw, "test1.xml");

  save_sandwich(sw, "test2.xml");
  return 0;
}
