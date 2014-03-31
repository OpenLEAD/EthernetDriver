#include <boost/test/unit_test.hpp>
#include <EthernetDrivers/Dummy.hpp>

using namespace EthernetDrivers;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    EthernetDrivers::DummyClass dummy;
    dummy.welcome();
}
