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
    void getHandleOutboundMsg(std::shared_ptr<UMessage> const uMsg) {
        handlerClient->handleOutboundMsg(uMsg);
    }
    void getProcessMessage(std::unique_ptr<SomeipHandler::QItem>& item) {
        handlerClient->processMessage(std::move(item));
    }
    std::unique_ptr<SomeipHandler::QItem> createQItem(HandlerMsgType type, unsigned long messageData, std::shared_ptr<void> messagePtr) {
        return std::make_unique<SomeipHandler::QItem>(type, messageData, messagePtr);
    }

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

/**
 * @brief Unit test to verify that no exception is thrown for onSubscriptionStatus when the status is successful
 */
TEST_F(SomeipHandlerClientTests, TestOnSubscriptionStatus_Successful) {
    vsomeip_v3::service_t service = vsomeip_v3::service_t(0x1234);
    vsomeip_v3::instance_t instance = vsomeip_v3::instance_t(0x1111);
    vsomeip_v3::eventgroup_t eventGroup = vsomeip_v3::eventgroup_t(0x2222);
    vsomeip_v3::event_t event = vsomeip_v3::event_t(0x3333);
    uint16_t status = 0;

    EXPECT_NO_THROW(handlerClient->onSubscriptionStatus(service, instance, eventGroup, event, status));
}

// /**
//  * @brief Test to ensure the SomeipHandler handleOutboundMsg method executes properly.
//  */
// TEST_F(SomeipHandlerClientTests, HandleOutboundMsgTest) {
//     auto uMsg = std::make_shared<UMessage>(g_messageHandler);

//     EXPECT_CALL(mockSomeipInterface, send(::testing::_)).Times(1);

//     getHandleOutboundMsg(uMsg);
// }

// /**
//  * @brief Test to ensure the SomeipHandler quit method executes properly.
//  */
// TEST_F(SomeipHandlerClientTests, QuitTest) {
//     EXPECT_CALL(mockSomeipInterface, stop()).Times(1);

//     handlerClient->quit();
// }

/**
 * @brief Test to ensure the SomeipHandler onAvailability method sets isServiceAvailable_ to true when service is available.
 */
TEST_F(SomeipHandlerClientTests, OnAvailabilityServiceAvailableTest) {
    vsomeip_v3::service_t service = vsomeip_v3::service_t(0x1234);
    vsomeip_v3::instance_t instance = vsomeip_v3::instance_t(0x1111);
    bool is_available = true;

    handlerClient->onAvailability(service, instance, is_available);

}

/**
 * @brief Test to ensure the SomeipHandler onAvailability method does not change isServiceAvailable_ when service is not available.
 */
TEST_F(SomeipHandlerClientTests, OnAvailabilityServiceNotAvailableTest) {
    vsomeip_v3::service_t service = vsomeip_v3::service_t(0x1234);
    vsomeip_v3::instance_t instance = vsomeip_v3::instance_t(0x1111);
    bool is_available = false;

    handlerClient->onAvailability(service, instance, is_available);

}


// /**
//  * @brief Unit test to verify the behavior of ProcessMessage for Outbound type
//  */
// TEST_F(SomeipHandlerClientTests, ProcessMessage_Outbound) {
//     ON_CALL(mockRouterInterface, isStateRegistered())
//         .WillByDefault(Return(true));

//     ON_CALL(mockSomeipInterface, offerService(4660, 4369, '\0', 0))
//         .WillByDefault(Return());
//     std::shared_ptr<UMessage> sharedMessageHandler = std::make_shared<UMessage>(g_messageHandler);
//     auto outboundItem = createQItem(HandlerMsgType::Outbound, 0UL, sharedMessageHandler);

//     getProcessMessage(outboundItem);
// }

/**
 * @brief Unit test to verify the behavior of ProcessMessage for Inbound type
 */
TEST_F(SomeipHandlerClientTests, ProcessMessage_Inbound) {
    ON_CALL(mockRouterInterface, isStateRegistered())
        .WillByDefault(Return(true));

    ON_CALL(mockSomeipInterface, offerService(4660, 4369, '\0', 0))
        .WillByDefault(Return());

    std::shared_ptr<vsomeip::message> inboundMsg = vsomeip::runtime::get()->create_request();
    inboundMsg->set_service(0x1234);
    inboundMsg->set_instance(0x1111);
    inboundMsg->set_method(0x3456);
    inboundMsg->set_message_type(static_cast<vsomeip::message_type_e>(999));
    std::shared_ptr< vsomeip::payload > its_payload = vsomeip::runtime::get()->create_payload();
    std::vector< vsomeip::byte_t > its_payload_data;
    for (vsomeip::byte_t i=0; i<10; i++) {
        its_payload_data.push_back(i % 256);
    }
    its_payload->set_data(its_payload_data);
    inboundMsg->set_payload(its_payload);
    auto inboundItem = createQItem( HandlerMsgType::Inbound, 0UL, inboundMsg);
    EXPECT_NO_THROW(getProcessMessage(inboundItem));
}

/**
 * @brief Unit test to verify the behavior of ProcessMessage for Stop type
 */
TEST_F(SomeipHandlerClientTests, ProcessMessage_Stop) {
    ON_CALL(mockRouterInterface, isStateRegistered())
        .WillByDefault(Return(true));

    ON_CALL(mockSomeipInterface, offerService(4660, 4369, '\0', 0))
        .WillByDefault(Return());



    auto stopItem = createQItem(HandlerMsgType::Stop, 0UL, nullptr);


    EXPECT_NO_THROW(getProcessMessage(stopItem));
}

/**
 * @brief Unit test to verify the behavior of ProcessMessage for Default type
 */
TEST_F(SomeipHandlerClientTests, ProcessMessage_default) {
    ON_CALL(mockRouterInterface, isStateRegistered())
        .WillByDefault(Return(true));

    ON_CALL(mockSomeipInterface, offerService(4660, 4369, '\0', 0))
        .WillByDefault(Return());

    auto defaultItem = createQItem(static_cast<HandlerMsgType>(999), 0UL, nullptr);


    EXPECT_NO_THROW(getProcessMessage(defaultItem));
}

/**
 * @brief Unit test to verify the behavior of ProcessMessage for Stop type
 */
TEST_F(SomeipHandlerClientTests, ProcessMessage_OfferUResource) {
    ON_CALL(mockRouterInterface, isStateRegistered())
        .WillByDefault(Return(true));

    ON_CALL(mockSomeipInterface, offerService(4660, 4369, '\0', 0))
        .WillByDefault(Return());



    auto OfferUResourceItem = createQItem(HandlerMsgType::OfferUResource, 0UL, g_testUURI);


    EXPECT_NO_THROW(getProcessMessage(OfferUResourceItem));
}

/**
 * @brief Unit test to verify the behavior of ProcessMessage for InboundSubscriptionAck type
 */
TEST_F(SomeipHandlerClientTests, ProcessMessage_InboundSubscriptionAck) {
    auto subscriptionStatusPtr = std::make_shared<subscriptionStatus>();
    subscriptionStatusPtr->isSubscribed = true;  
    subscriptionStatusPtr->eventgroup = 0x123;  

    auto inboundSubscriptionAckItem = createQItem(HandlerMsgType::InboundSubscriptionAck, 0UL, subscriptionStatusPtr);
    EXPECT_NO_THROW(getProcessMessage(inboundSubscriptionAckItem));
    // Add more assertions to verify the behavior of handleInboundSubscriptionAck
}

/**
 * @brief Unit test to verify the behavior of ProcessMessage for InboundSubscription type
 */
TEST_F(SomeipHandlerClientTests, ProcessMessage_InboundSubscription) {
    auto subscriptionStatusPtr = std::make_shared<subscriptionStatus>();
    subscriptionStatusPtr->isSubscribed = true;  
    subscriptionStatusPtr->eventgroup = 0x123;  

    auto inboundSubscriptionItem = createQItem(HandlerMsgType::InboundSubscription, 0UL, subscriptionStatusPtr);
    EXPECT_NO_THROW(getProcessMessage(inboundSubscriptionItem));

}

// TEST_F(SomeipHandlerClientTests, HandleOutboundMsgPublishTest) {
//     auto const lType = UMessageType::UMESSAGE_TYPE_PUBLISH;
//     UAttributesBuilder builderHandler(*g_testUURI, g_uuidHandler, lType, g_priority);
//     UAttributes attributesHandler = builderHandler.build();

//     auto uMsg = std::make_shared<UMessage>(g_payloadHandler, attributesHandler);

//     getHandleOutboundMsg(uMsg);
// }

TEST_F(SomeipHandlerClientTests, HandleOutboundMsgRequestTest) {
    auto const lType = UMessageType::UMESSAGE_TYPE_REQUEST;
    UAttributesBuilder builderHandler(*g_testUURI, g_uuidHandler, lType, g_priority);
    UAttributes attributesHandler = builderHandler.build();

    auto uMsg = std::make_shared<UMessage>(g_payloadHandler, attributesHandler);

    getHandleOutboundMsg(uMsg);
}
TEST_F(SomeipHandlerClientTests, HandleOutboundMsgResponseTest) {
    auto const lType = UMessageType::UMESSAGE_TYPE_RESPONSE;
    UAttributesBuilder builderHandler(*g_testUURI, g_uuidHandler, lType, g_priority);
    UAttributes attributesHandler = builderHandler.build();

    auto uMsg = std::make_shared<UMessage>(g_payloadHandler, attributesHandler);

    getHandleOutboundMsg(uMsg);
}

TEST_F(SomeipHandlerClientTests, HandleOutboundMsgUnknownTypeTest) {
    auto const lType = static_cast<UMessageType>(999);
    UAttributesBuilder builderHandler(*g_testUURI, g_uuidHandler, lType, g_priority);
    UAttributes attributesHandler = builderHandler.build();

    auto uMsg = std::make_shared<UMessage>(g_payloadHandler, attributesHandler);

    getHandleOutboundMsg(uMsg);
}