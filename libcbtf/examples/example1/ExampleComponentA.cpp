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
class __attribute__ ((visibility ("hidden"))) ExampleComponentA :
    public Component
{
public:
    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(reinterpret_cast<Component*>(new ExampleComponentA()));
    }
private:
    /** Default constructor. */
    ExampleComponentA() :
        Component(Type(typeid(ExampleComponentA)), Version(0, 0, 2))
    {
        declareInput<int>(
            "in", boost::bind(&ExampleComponentA::inHandler, this, _1)
            );
        declareOutput<int>("out");
    }
    /** Handler for the "in" input.*/
    void inHandler(const int& in)

    { printf("in an instance of ExampleComponentA, in=%d\n", in); 
      emitOutput<int>("out", in / 2 ); }
}; // class ExampleComponentA
KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(ExampleComponentA)



