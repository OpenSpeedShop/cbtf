#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <stdio.h>
#include <ltdl.h>
#include <stdexcept>
#include <typeinfo>
#include <unistd.h>
#include <stdio.h>

#include <KrellInstitute/CBTF/BoostExts.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/ValueSink.hpp>
#include <KrellInstitute/CBTF/ValueSource.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;

int main() {

//Component::registerPlugin("ExampleComponentA.la")
Component::registerPlugin(boost::filesystem::path(BUILDDIR) / "ExampleComponentA");
std::set<Type> available_types1 = Component::getAvailableTypes();
//Component::registerPlugin("ExampleComponentB.la");
Component::registerPlugin(boost::filesystem::path(BUILDDIR) / "ExampleComponentB");
std::set<Type> available_types2 = Component::getAvailableTypes();


// Test component versioning
std::set<Version> available_versions =
       Component::getAvailableVersions(Type("ExampleComponentA"));

printf("We should have ONE available types of components, number of types=%ld\n", available_types1.size());
printf("We should have TWO available types of components, number of types=%ld\n", available_types2.size());


Component::Instance instance_of_a1;
Component::Instance instance_of_a2;
Component::Instance instance_of_a3;

instance_of_a1 = Component::instantiate(Type("ExampleComponentA"));

std::string a1_build_str = instance_of_a1->getBuildString();
printf("Build String for A1=%s\n", a1_build_str.c_str());


std::map<std::string, Type> inputs = instance_of_a1->getInputs();
printf("We should have ONE input for A1, number of inputs=%ld\n", inputs.size());




instance_of_a2 = Component::instantiate(Type("ExampleComponentA"));
instance_of_a3 = Component::instantiate(Type("ExampleComponentA"));

Component::Instance instance_of_b1;
Component::Instance instance_of_b2;

instance_of_b1 = Component::instantiate(Type("ExampleComponentB"));
instance_of_b2 = Component::instantiate(Type("ExampleComponentB"));


Component::connect( instance_of_a1, "out", instance_of_a2, "in");
Component::connect( instance_of_a2, "out", instance_of_a3, "in");
Component::connect( instance_of_a2, "out", instance_of_b1, "in");
Component::connect( instance_of_a3, "out", instance_of_b2, "in");
Component::connect( instance_of_b1, "out", instance_of_b2, "in");

boost::shared_ptr<ValueSource<int> > input_value = ValueSource<int>::instantiate();
boost::shared_ptr<ValueSink<int> > output_value = ValueSink<int>::instantiate();
Component::Instance input_value_component = boost::reinterpret_pointer_cast<Component>(input_value);
Component::Instance output_value_component = boost::reinterpret_pointer_cast<Component>(output_value);
Component::connect(input_value_component, "value", instance_of_a1, "in");
Component::connect(instance_of_b2, "out", output_value_component, "value");
*input_value = 10;
if (*output_value == 8) {
  printf("Example Passes\n");
} else {
  printf("Example Fails\n");
}



}
