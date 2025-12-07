#include <ns3/test.h>

namespace ns3 {

class PlaceholderTest : public TestCase
{
public:
    PlaceholderTest() : TestCase("Placeholder Test") {}
    virtual ~PlaceholderTest() {}

private:
    void DoRun() override
    {
        NS_TEST_ASSERT_MSG_EQ(true, true, "Placeholder test should pass");
    }
};

static PlaceholderTest g_placeholderTest;

} // namespace ns3
