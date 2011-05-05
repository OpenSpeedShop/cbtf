#include <boost/bind.hpp>
#include <typeinfo>
#include <stdio.h>


#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;

/**
 *  * Component type used by the unit test for the Component class.
 *   */
class __attribute__ ((visibility ("hidden"))) ExampleComponentB :
    public Component
{
public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(reinterpret_cast<Component*>(new ExampleComponentB()));
    }
private:
    /** Default constructor. */
    ExampleComponentB() :
        Component(Type(typeid(ExampleComponentB)), Version(0, 0, 2))
    {
        declareInput<int>(
            "in", boost::bind(&ExampleComponentB::inHandler, this, _1)
            );
        declareOutput<int>("out");
    }
    /** Handler for the "in" input.*/
    void inHandler(const int& in)
    { printf("in an instance of ExampleComponentB, in=%d\n", in);
      emitOutput<int>("out", in * 2 ); }
}; // class ExampleComponentB
KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(ExampleComponentB)



