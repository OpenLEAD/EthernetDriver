#include <iostream>
#include <EthernetDrivers/Dummy.hpp>

int main(int argc, char** argv)
{
	EthernetDrivers::DummyClass dummyClass;
	dummyClass.welcome();

	return 0;
}
