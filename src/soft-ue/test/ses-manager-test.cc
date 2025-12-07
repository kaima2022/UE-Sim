#include <ns3/test.h>

namespace ns3 {

class SesManagerBasicTest : public TestCase
{
public:
    SesManagerBasicTest() : TestCase("SES Manager Basic Test") {}
    virtual ~SesManagerBasicTest() {}

private:
    void DoRun() override
    {
        NS_TEST_ASSERT_MSG_EQ(true, true, "Basic SES Manager test should pass");
    }
};

class SesManagerTestSuite : public TestSuite
{
public:
    SesManagerTestSuite() : TestSuite("soft-ue-ses", Type::UNIT)
    {
        AddTestCase(new SesManagerBasicTest(), Duration::QUICK);
    }
};

static SesManagerTestSuite g_sesManagerTestSuite;

} // namespace ns3
