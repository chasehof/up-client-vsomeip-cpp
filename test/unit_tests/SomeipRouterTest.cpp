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
#include "MockSomeipInterface.hpp"
#include "MockSomeipRouterInterface.hpp"
#include "MockSomeipHandler.hpp"

using ::testing::Return;
using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;
using ::testing::NiceMock;

class MockUListener : public UListener {
public:
    MOCK_METHOD(UStatus, onReceive, (UMessage &message), (const, override));
};

std::shared_ptr<UUri> uRISource = buildUURI();

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

    std::shared_ptr<SomeipHandler> getNewHandler(HandlerType type,
                                                 const UEntity &uEntityInfo,
                                                 const UAuthority &uAuthorityInfo) {
        return router->newHandler(type, uEntityInfo, uAuthorityInfo);
    }

    void addHandler(std::shared_ptr<SomeipHandler> mockHandler, uint16_t mockHandlerKey) {
        router->handlers_[mockHandlerKey] = mockHandler;
    }

    void getOfferServicesAndEvents(std::shared_ptr<UUri> uriPtr) {
        router->offerServicesAndEvents(uriPtr);
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
    builder.setSource(*uRISource);
    UAttributes attributes = builder.build();
    uint8_t buffer[10] = {0}; 
    UPayload payload(buffer, sizeof(buffer), UPayloadType::VALUE);

    // Build the UMessage
    UMessage umsg(payload, attributes);

    return umsg;
}

/**
 *  @brief Verify init() successfully initializes a SomeipRouter object.
 */
TEST_F(SomeipRouterTests, routeInboundMsgTest) {
    uprotocol::utransport::UMessage umsg;
    
    EXPECT_CALL(mockListener,  onReceive(::testing::_)).Times(1);
    EXPECT_TRUE(router->routeInboundMsg(umsg));
}

/**
 *  @brief Verify that a publish or response outbound message is not routed if the handler does not exist in the list.
 */
TEST_F(SomeipRouterTests, routeOutboundMsgPublishAndResponseNullTest) {
    uprotocol::utransport::UMessage umsg = buildUMessage(UMessageType::UMESSAGE_TYPE_PUBLISH,
                                                         UPriority::UPRIORITY_CS4);

    EXPECT_FALSE(router->routeOutboundMsg(umsg));
    EXPECT_FALSE(router->routeOutboundMsg(umsg));
}

/**
 *  @brief Test that getOfferServicesAndEvents does not manipulate a SomeipHandler if there is no corresponding
 *  entity ID.
 */
TEST_F(SomeipRouterTests, offerServicesAndEventsTest) {
    MockSomeipInterface mockSomeipInterface;
    MockSomeipRouterInterface mockSomeipRouterInterface;

    std::string const uEntityName        = "0x1102";
    uint32_t const uEntityVersionMajor   = 0x1; //Major Version
    uint32_t const uEntityVersionMinor   = 0x0; //Minor Version
    std::string const uAuthorityIp       = "172.17.0.1";
    uint16_t const uResourceId           = 0x0102; //Method ID
    std::string const uResourceName      = "rpc";
    std::string const uResourceInstance  = "0x0102";
    std::shared_ptr<UUri> uriPtr = std::make_shared<UUri>();
    UEntity uEntity;
    UAuthority uAuthority;
    UResource uResource;
 
    //uEntity.set_id(uEntityId);
    uEntity.set_name(uEntityName.c_str());
    std::cout << "Entity Name: " << uEntity.name() << std::endl;
    uEntity.set_version_major(uEntityVersionMajor);
    uEntity.set_version_minor(uEntityVersionMinor);
    uriPtr->mutable_entity()->CopyFrom(uEntity);
 
    uResource.set_id(uResourceId);
    uResource.set_name(uResourceName.c_str());
    std::cout << "Resource Name: " << uResource.name() << std::endl;
    uResource.set_instance(uResourceInstance);
 
    uriPtr->mutable_resource()->CopyFrom(uResource);

    std::shared_ptr<MockSomeipHandler> mockHandler = std::make_shared<MockSomeipHandler>(mockSomeipInterface,
    mockSomeipRouterInterface,
    HandlerType::Client,
    uEntity,
    uAuthority,
    0x0102,
    DEFAULT_PRIORITY_LEVELS);

    uint16_t mockHandlerKey = 0x1102;
    addHandler(mockHandler, mockHandlerKey);
    
    EXPECT_NO_THROW(getOfferServicesAndEvents(uriPtr));
}