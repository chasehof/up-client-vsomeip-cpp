#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <vector>
#include "SomeipHandler.hpp"
#include "MessageTranslator.hpp"
#include "mock/MockSomeipInterface.hpp"
#include "mock/MockSomeipRouterInterface.hpp"
#include <VsomeipUTransport.hpp>
#include <up-cpp/transport/builder/UAttributesBuilder.h>
#include <up-core-api/uri.pb.h>
#include <up-cpp/uuid/factory/Uuidv8Factory.h>
#include "mock/UURIHelper.hpp"

using ::testing::Return;
using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;
using ::testing::NiceMock;
using namespace up::vsomeip_client;

/**
 *  @brief Assets needed to make a message.
 */
auto g_uuidHandler = Uuidv8Factory::create();
auto const g_testUURI = buildUURI();
auto const g_priority = UPriority::UPRIORITY_CS4;
auto const g_publishType = UMessageType::UMESSAGE_TYPE_PUBLISH;
UAttributesBuilder g_builderHandler(*g_testUURI, g_uuidHandler, g_publishType, g_priority);
UAttributes g_attributesHandler = g_builderHandler.build();

/**
 *  @brief payload for message.
 */
static uint8_t g_data[4] = "100";
UPayload g_payloadHandler(g_data, sizeof(g_data), UPayloadType::VALUE);

/**
 *  @brief UMessage used for testing.
 */
UMessage g_messageHandler(g_payloadHandler, g_attributesHandler);

/**
 *  @brief Parameters for someip calls.
 */
service_t const g_service = 0x1234;
instance_t const g_instance = 0x1111;

/**
 *  @brief SomeipHandler test fixture for a SomeipHandler Server type.
 */
class SomeipHandlerServerTests : public ::testing::Test {
protected:
    ::testing::NiceMock<MockSomeipInterface> mockSomeipInterface;
    ::testing::NiceMock<MockSomeipRouterInterface> mockRouterInterface;
    std::unique_ptr<SomeipHandler> handlerServer;
    UEntity uEntity;
    UAuthority uAuthority;

    /**
     *  @brief Setup for SomeipHandler. Initializes required variables.
     */
    void SetUp() override {
        handlerServer = std::make_unique<SomeipHandler>(mockSomeipInterface,
                                                        mockRouterInterface,
                                                        HandlerType::Server,
                                                        uEntity,
                                                        uAuthority,
                                                        g_instance,
                                                        2U);
    }
 
    /**
     *  @brief Teardown for SomeipHandler.
     */
    void TearDown() override {
        handlerServer->quit();
    }
};

/**
 *  @brief SomeipHandler test fixture for a SomeipHandler client type.
 */
class SomeipHandlerClientTests : public ::testing::Test {
protected:
    ::testing::NiceMock<MockSomeipInterface> mockSomeipInterface;
    ::testing::NiceMock<MockSomeipRouterInterface> mockRouterInterface;
    std::unique_ptr<SomeipHandler> handlerClient;
    UEntity uEntity;
    UAuthority uAuthority;
    
    /**
     *  @brief Setup for SomeipHandler. Initializes required variables.
     */
    void SetUp() override {
        handlerClient = std::make_unique<SomeipHandler>(mockSomeipInterface,
                                                        mockRouterInterface,
                                                        HandlerType::Client,
                                                        uEntity,
                                                        uAuthority,
                                                        g_instance,
                                                        2U);
    }
    
    /**
     *  @brief Teardown for SomeipHandler.
     */
    void TearDown() override {
        handlerClient->quit();
    }
};

/**
 *  @brief Test to ensure the SomeipHandler constructor executes properly for client handlers.
 */
TEST(SomeipHandlerStandaloneTests, SomeipHandlerClientConstructorTest) {
    ::testing::NiceMock<MockSomeipInterface> mockSomeipInterface;
    ::testing::NiceMock<MockSomeipRouterInterface> mockRouterInterface;
    std::unique_ptr<SomeipHandler> handlerClient;
    UEntity uEntity;
    UAuthority uAuthority;

    ON_CALL(mockRouterInterface, isStateRegistered()).WillByDefault(Return(true));
    EXPECT_CALL(mockSomeipInterface, registerAvailabilityHandler(0,
                                                                 0,
                                                                 ::testing::_,
                                                                 ::testing::_,
                                                                 ::testing::_)).Times(1);
    EXPECT_CALL(mockSomeipInterface, requestService(0, 0, ::testing::_, ::testing::_)).Times(1);
    EXPECT_CALL(mockSomeipInterface, offerService(0, 0, ::testing::_, ::testing::_)).Times(0);

    handlerClient = std::make_unique<SomeipHandler>(mockSomeipInterface,
                                                    mockRouterInterface,
                                                    HandlerType::Client,
                                                    uEntity,
                                                    uAuthority,
                                                    g_instance,
                                                    2U);
}

/**
 *  @brief Test to ensure the SomeipHandler constructor executes properly for server handlers.
 */
TEST(SomeipHandlerStandaloneTests, SomeipHandlerServerConstructorTest) {
    ::testing::NiceMock<MockSomeipInterface> mockSomeipInterface;
    ::testing::NiceMock<MockSomeipRouterInterface> mockRouterInterface;
    std::unique_ptr<SomeipHandler> handlerServer;
    UEntity uEntity;
    UAuthority uAuthority;

    EXPECT_CALL(mockSomeipInterface, registerAvailabilityHandler(0,
                                                                 0,
                                                                 ::testing::_,
                                                                 ::testing::_,
                                                                 ::testing::_)).Times(0);
    EXPECT_CALL(mockSomeipInterface, requestService(0, 0, ::testing::_, ::testing::_)).Times(0);
    EXPECT_CALL(mockSomeipInterface, offerService(0, 0, ::testing::_, ::testing::_)).Times(1);

    handlerServer = std::make_unique<SomeipHandler>(mockSomeipInterface,
                                                    mockRouterInterface,
                                                    HandlerType::Server,
                                                    uEntity,
                                                    uAuthority,
                                                    g_instance,
                                                    2U);
}

/**
 * @brief Verify the behavior of onSubscription when the client is subscribed to a service.
 */
TEST_F(SomeipHandlerClientTests, onSubscriptionTest) {
    client_t client = 123;
    secClientType secClient;
    std::string str = "test_subscription";
    bool isSubscribed = true;


    bool result = handlerClient->onSubscription(client, &secClient, str, isSubscribed);
    EXPECT_TRUE(result);
}

/**
 * @brief Verify the behavior of onSubscription when the client is not subscribed to a service.
 */
TEST_F(SomeipHandlerClientTests, onSubscriptionNotSubscribedTest) {
    client_t client = 123;
    secClientType secClient;
    std::string str = "test_subscription";
    bool isSubscribed = false;

    bool result = handlerClient->onSubscription(client, &secClient, str, isSubscribed);
    EXPECT_TRUE(result);
}

// TEST_F(SomeipHandlerClientTests, OnSubscriptionStatus_SubscribedStatus_PostsMessageToQueue) {
//     // Arrange
//     service_t service = 0x1234;
//     instance_t instance = 0x1111;
//     eventgroup_t eventGroup = 0x5678;
//     event_t event = 0x0001;
//     uint16_t status = 0;

//     // Act
//     handlerClient->onSubscriptionStatus(service, instance, eventGroup, event, status);


// }

// TEST_F(SomeipHandlerClientTests, OnSubscriptionStatus_UnsubscribedStatus_PostsMessageToQueue) {
//     // Arrange
//     service_t service = 0x1234;
//     instance_t instance = 0x1111;
//     eventgroup_t eventGroup = 0x5678;
//     event_t event = 0x0001;
//     uint16_t status = 1;

//     // Act
//     handlerClient->onSubscriptionStatus(service, instance, eventGroup, event, status);

// }

// TEST_F(SomeipHandlerClientTests, OnSubscriptionStatus_ExceptionCaught_LogsErrorMessage) {
//     // Arrange
//     service_t service = 0x1234;
//     instance_t instance = 0x1111;
//     eventgroup_t eventGroup = 0x5678;
//     event_t event = 0x0001;
//     uint16_t status = 0;

//     // Act
//     handlerClient->onSubscriptionStatus(service, instance, eventGroup, event, status);

// }