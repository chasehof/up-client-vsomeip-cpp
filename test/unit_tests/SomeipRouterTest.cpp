#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <vector>
#include "SomeipRouter.hpp"
#include "MessageTranslator.hpp"
#include "mock/UURIHelper.hpp"
#include <VsomeipUTransport.hpp>
#include <up-cpp/transport/builder/UAttributesBuilder.h>
#include <up-core-api/uri.pb.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include <up-cpp/uuid/serializer/UuidSerializer.h>


using ::testing::Return;
using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;
using ::testing::NiceMock;

class MockUListener : public UListener
{
    UStatus onReceive(UMessage &message) const {
        std::ignore = message;
        UStatus status;
        status.set_code(UCode::OK);
        return status;
    }
};

/**
 *  @brief SomeipRouter test fixture.
 */
class SomeipRouterTests : public testing::Test {
protected:
    /**
     *  @brief Mock UListener object needed to initialize a SomeipRouter.
     */
    MockUListener mockListener;
    /**
     *  @brief Message for routing.
     */
    UMessage umsg;
    /**
     *  @brief SomeipRouter object for testing.
     */
    std::shared_ptr<SomeipRouter> router;

    /**
     *  @brief Setup for SomeipRouter.
     */
    void SetUp() override {
    router = std::make_shared<SomeipRouter>(mockListener);
    }

    /**
     *  @brief Teardown for SomeipRouter.
     */
    void TearDown() override {

    }
};

/**
 *  @brief Build a UMessage needed for testing.
 */
UMessage buildUMessage(UMessageType type, UPriority priority) {
    // Setup
    auto uuid = Uuidv8Factory::create();
    std::shared_ptr<UUri> uRI = buildUURI();
    UAttributesBuilder builder(*uRI, uuid, type, priority);
    builder.setSink(*uRI);
    UAttributes attributes = builder.build();
    uint8_t buffer[10] = {0}; 
    UPayload payload(buffer, sizeof(buffer), UPayloadType::VALUE);

    // Build the UMessage
    UMessage umsg(payload, attributes);

    return umsg;
}

// /**
//  *  @brief Verify init() successfully initializes a SomeipRouter object.
//  */
// TEST_F(SomeipRouterTests, TestInit) {
//   
//     EXPECT_TRUE(router->init());
// }