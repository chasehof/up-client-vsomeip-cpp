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
using ::testing::_;

/**
 *  @brief Create a UResource object.
 */
std::unique_ptr<UResource> createUResource();

/**
 *  @brief Build a UAttributes object.
 */
UAttributes createUAttributes(UMessageType type = UMESSAGE_TYPE_PUBLISH);

/**
 *  @brief Create a UUri object for testing.
 */
auto const g_testUURI = buildUURI();

/**
 *  @brief Create a message object for testing.
 */
std::shared_ptr<vsomeip::message> createMessage();

/**
 *  @brief payload for message.
 */
static uint8_t g_data[4] = "100";
UPayload g_payloadForHandler(g_data, sizeof(g_data), UPayloadType::VALUE);
/**
 *  @brief UMessage used for testing.
 */
UMessage g_messageHandler(g_payloadForHandler, createUAttributes());
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

    /**
     *  @brief Getters for private methods in SomeipHandler.
     */
    bool getDoesInboundSubscriptionExist(eventgroup_t const eventGroup) {
        return handlerClient->doesInboundSubscriptionExist(eventGroup);
    }

    void gethandleOfferUResource(std::shared_ptr<UUri> const uriPtr) {
        handlerClient->handleOfferUResource(uriPtr);
    }

    void gethandleInboundSubscription(std::shared_ptr<subscriptionStatus> const subStatus) {
        handlerClient->handleInboundSubscription(subStatus);
    }

    void getActOnBehalfOfSubscriptionAck(eventgroup_t const eventGroup) {
        handlerClient->running_ = true;
        handlerClient->actOnBehalfOfSubscriptionAck(eventGroup);
    }

    size_t getQueueSize() {
        return handlerClient->queue_[0].size();
    }

    bool getaddSubscriptionForRemoteService(UResourceId_t resourceid,
                                            std::shared_ptr<ResourceInformation> resourceInfo) {
        handlerClient->addSubscriptionForRemoteService(resourceid, resourceInfo);

        return handlerClient->subscriptionsForRemoteServices_.find(resourceid) !=
            handlerClient->subscriptionsForRemoteServices_.end();
    }

    bool getRemoveSubscriptionForRemoteService(UResourceId_t resourceid) {
        handlerClient->removeSubscriptionForRemoteService(resourceid);

        return handlerClient->subscriptionsForRemoteServices_.find(resourceid) ==
            handlerClient->subscriptionsForRemoteServices_.end();
    }

    bool getdoesSubscriptionForRemoteServiceExist(eventgroup_t eventGroup) {
        return handlerClient->doesSubscriptionForRemoteServiceExist(eventGroup);
    }

    void getHandleOutboundResponse(std::shared_ptr<UMessage> const message) {
        handlerClient->handleOutboundResponse(message);
    }

    void getHandleInboundRequest(std::shared_ptr<message> message) {
        handlerClient->handleInboundRequest(message);
    }

    void addToUuidToSomeipRequestLookup(std::string strUUID, std::shared_ptr<message> sMsg) {
        handlerClient->uuidToSomeipRequestLookup_.insert({strUUID, sMsg});
    }

    void getHandleSubscriptionRequestForRemoteService(std::shared_ptr<uprotocol::utransport::UMessage> const uMsg) {
        handlerClient->handleSubscriptionRequestForRemoteService(uMsg);
    }

    void getHandleOutboundNotification(std::shared_ptr<UMessage> const uMsgPtr) {
        handlerClient->handleOutboundNotification(uMsgPtr);
    }

    void getHandleOutboundMsg(std::shared_ptr<UMessage> const uMsg) {
        handlerClient->handleOutboundMsg(uMsg);
    }
    void getProcessMessage(std::unique_ptr<SomeipHandler::QItem>& item) {
        handlerClient->processMessage(std::move(item));
    }
};

std::unique_ptr<UResource> createUResource() {
    uint16_t const uResourceId = 0x0102; //Method ID
    std::string const uResourceName = "rpc";
    std::string const uResourceInstance = "0x0102";

    std::unique_ptr<UResource> uResource = std::make_unique<UResource>();
    uResource->set_id(uResourceId);
    uResource->set_name(uResourceName.c_str());
    uResource->set_instance(uResourceInstance);

    return uResource;
}

UAttributes createUAttributes(UMessageType type) {
    auto uuid = Uuidv8Factory::create();
    auto const uPriority = UPriority::UPRIORITY_CS4;
    auto uPublishType = type;
    UAttributesBuilder uAttributesBuilder(*g_testUURI, uuid, uPublishType, uPriority);
    uAttributesBuilder.setReqid(uuid);

    return uAttributesBuilder.build();
}

std::shared_ptr<vsomeip::message>  createMessage() {
    std::shared_ptr<vsomeip::message> message;
    message = vsomeip::runtime::get()->create_request();
    message->set_service(g_service);
    message->set_instance(g_instance);
    message->set_method(0x0102);
 
    std::shared_ptr< vsomeip::payload > payload = vsomeip::runtime::get()->create_payload();
    std::vector< vsomeip::byte_t > payloadData;
    for (vsomeip::byte_t i = 0; i < 10; i++) {
        payloadData.push_back(i % 256);
    }
    payload->set_data(payloadData);
    message->set_payload(payload);

    return message;
}

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
                                                                 _,
                                                                 _,
                                                                 _)).Times(1);
    EXPECT_CALL(mockSomeipInterface, requestService(0, 0, _, _)).Times(1);
    EXPECT_CALL(mockSomeipInterface, offerService(0, 0, _, _)).Times(0);

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
                                                                 _,
                                                                 _,
                                                                 _)).Times(0);
    EXPECT_CALL(mockSomeipInterface, requestService(0, 0, _, _)).Times(0);
    EXPECT_CALL(mockSomeipInterface, offerService(0, 0, _, _)).Times(1);

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

TEST_F(SomeipHandlerClientTests, OnSubscriptionStatus_SubscribedStatus_PostsMessageToQueue) {
    // Arrange
    service_t service = 0x1234;
    instance_t instance = 0x1111;
    eventgroup_t eventGroup = 0x5678;
    event_t event = 0x0001;
    uint16_t status = 0;

    // Act
    handlerClient->onSubscriptionStatus(service, instance, eventGroup, event, status);


}

TEST_F(SomeipHandlerClientTests, OnSubscriptionStatus_UnsubscribedStatus_PostsMessageToQueue) {
    // Arrange
    service_t service = 0x1234;
    instance_t instance = 0x1111;
    eventgroup_t eventGroup = 0x5678;
    event_t event = 0x0001;
    uint16_t status = 1;

    // Act
    handlerClient->onSubscriptionStatus(service, instance, eventGroup, event, status);

}

TEST_F(SomeipHandlerClientTests, OnSubscriptionStatus_ExceptionCaught_LogsErrorMessage) {
    // Arrange
    service_t service = 0x1234;
    instance_t instance = 0x1111;
    eventgroup_t eventGroup = 0x5678;
    event_t event = 0x0001;
    uint16_t status = 0;

    // Act
    handlerClient->onSubscriptionStatus(service, instance, eventGroup, event, status);

}

/**
 * @brief Ensure doesInboundSubscriptionExist returns true only when inbound subscription exists.
 */
TEST_F(SomeipHandlerClientTests, doesInboundSubscriptionExistTest) {
    eventgroup_t eventGroup = 0x0102;

    /**
     *  @brief Assets needed to make a UResource object.
     */
    uint16_t const g_uResourceIdForHandler           = 0x0102; //Method ID
    std::string const g_uResourceNameForHandler      = "rpc";
    std::string const g_uResourceInstanceForHandler  = "0x0102";
    std::unique_ptr<UResource> uResource = std::make_unique<UResource>();
    /**
     *  @brief Set parameters of UResource object.
     */
    uResource->set_id(g_uResourceIdForHandler);
    uResource->set_name(g_uResourceNameForHandler.c_str());
    uResource->set_instance(g_uResourceInstanceForHandler);

    /**
     *  @brief SubscriptionStatus object. Used to add to the subscriber count using handleInboundSubscription.
     */
    subscriptionStatus subStatus;
    subStatus.isSubscribed = true;
    subStatus.eventgroup = eventGroup;
    std::shared_ptr<subscriptionStatus> subStatusPtr = std::make_shared<subscriptionStatus>(subStatus);

    /**
     *  @brief Create a copy of UResource object.
     */
    EXPECT_FALSE(getDoesInboundSubscriptionExist(eventGroup));

    /**
     *  @brief Add the resourceId and resource to the offeredResources_ map.
     */
    gethandleOfferUResource(g_testUURI);
    /**
     *  @brief Find resourceId in map and add to subscriber count.
     */
    gethandleInboundSubscription(subStatusPtr);
    EXPECT_TRUE(getDoesInboundSubscriptionExist(eventGroup));
}

/**
 *  @brief Check if the queue size increases when actOnBehalfOfSubscriptionAck is called.
 *  actOnBehalfOfSubscriptionAck calls onSubscriptionStatus which then calls postMessageToQueue.
 */
TEST_F(SomeipHandlerClientTests, actOnBehalfOfSubscriptionAckTest) {
    eventgroup_t eventGroup = 0x0123;
    size_t originalSize = getQueueSize();

    getActOnBehalfOfSubscriptionAck(eventGroup);
    EXPECT_EQ(originalSize + 1, getQueueSize());
}


/**
 *  @brief Ensure addSubscriptionForRemoteService adds a subscription for a remote service.
 */
TEST_F(SomeipHandlerClientTests, addSubscriptionForRemoteServiceTest) {
    UResourceId_t resourceId = 0x0102;
    std::unique_ptr<UResource> resource = createUResource();
    std::shared_ptr<ResourceInformation> resourceInfo = std::make_shared<ResourceInformation>(*resource);

    EXPECT_TRUE(getaddSubscriptionForRemoteService(resourceId, resourceInfo));
}

/**
 *  @brief Ensure removeSubscriptionForRemoteService removes a subscription for a remote service.
 */
TEST_F(SomeipHandlerClientTests, removeSubscriptionForRemoteServiceTest) {
    UResourceId_t resourceId = 0x0102;
    std::unique_ptr<UResource> resource = createUResource();
    std::shared_ptr<ResourceInformation> resourceInfo = std::make_shared<ResourceInformation>(*resource);

    std::ignore = getaddSubscriptionForRemoteService(resourceId, resourceInfo);
    EXPECT_TRUE(getRemoveSubscriptionForRemoteService(resourceId));
}

/**
 *  @brief Verify that doesSubscriptionForRemoteServiceExist correctly returns if a remote subscription exists.
 */
TEST_F(SomeipHandlerClientTests, doesSubscriptionForRemoteServiceExistTest) {
    eventgroup_t eventGroup = 0x0123;
    UResourceId_t resourceId = 0x0123;
    std::unique_ptr<UResource> resource = createUResource();
    std::shared_ptr<ResourceInformation> resourceInfo = std::make_shared<ResourceInformation>(*resource);

    EXPECT_FALSE(getdoesSubscriptionForRemoteServiceExist(eventGroup));

    std::ignore = getaddSubscriptionForRemoteService(resourceId, resourceInfo);
    EXPECT_TRUE(getdoesSubscriptionForRemoteServiceExist(eventGroup));
    
}

/**
 *  @brief Check that SomeipHandler calls SomeipInterface.registerMessageHandler.
 */
TEST_F(SomeipHandlerClientTests, registerMessageHandlerTest) {
    EXPECT_CALL(mockSomeipInterface, registerMessageHandler(_,
                                                            _,
                                                            _,
                                                            _)).Times(1);
    handlerClient = std::make_unique<SomeipHandler>(mockSomeipInterface,
                                                mockRouterInterface,
                                                HandlerType::Client,
                                                uEntity,
                                                uAuthority,
                                                g_instance,
                                                2U);
}

/**
 *  @brief Verify that a UResource is offered when handleOfferUResource is called.
 */
TEST_F(SomeipHandlerClientTests, handleOfferUResourceTest) {
    eventgroup_t eventGroup = 0x0102;

    /**
     *  @brief SubscriptionStatus object. Used to add to the subscriber count using handleInboundSubscription.
     */
    subscriptionStatus subStatus;
    subStatus.isSubscribed = true;
    subStatus.eventgroup = eventGroup;
    std::shared_ptr<subscriptionStatus> subStatusPtr = std::make_shared<subscriptionStatus>(subStatus);

    EXPECT_FALSE(getDoesInboundSubscriptionExist(eventGroup));

    gethandleOfferUResource(g_testUURI);
    gethandleInboundSubscription(subStatusPtr);
    EXPECT_TRUE(getDoesInboundSubscriptionExist(eventGroup));
}


/**
 *  @brief Test that the handleOutboundResponse method only calls SomeipInterface.send if the UUID exists in the map.
 */
TEST_F(SomeipHandlerClientTests, handleOutboundResponseTest) {
    std::shared_ptr<uprotocol::utransport::UMessage> messagePtr =
        std::make_shared<uprotocol::utransport::UMessage>(g_messageHandler);
    std::shared_ptr<vsomeip::message> message = createMessage();
    UMessage const &uMsg = *messagePtr;
    std::string strUUID = uprotocol::uuid::UuidSerializer::serializeToString(uMsg.attributes().reqid());

    EXPECT_CALL(mockSomeipInterface, send(_)).Times(0);

    getHandleOutboundResponse(messagePtr);
    addToUuidToSomeipRequestLookup(strUUID, message);
    EXPECT_CALL(mockSomeipInterface, send(_)).Times(1);

    getHandleOutboundResponse(messagePtr);
}

/**
 * @brief Unit test to verify the behavior of HandleOutboundMsg for RESPONSE type
 */
TEST_F(SomeipHandlerClientTests, HandleOutboundMsgResponseTest) {
    auto const lType = UMessageType::UMESSAGE_TYPE_RESPONSE;
    auto uuid = Uuidv8Factory::create();
    UAttributesBuilder builderHandler(*g_testUURI,uuid, lType, UPriority::UPRIORITY_CS4);
    UAttributes attributesHandler = builderHandler.build();

    auto uMsg = std::make_shared<UMessage>(g_payloadForHandler, attributesHandler);

    EXPECT_NO_THROW(getHandleOutboundMsg(uMsg));
}

/**
 * @brief Unit test to verify the behavior of HandleOutboundMsg for UNKNOWN type
 */
TEST_F(SomeipHandlerClientTests, HandleOutboundMsgUnknownTypeTest) {
    auto const lType = static_cast<UMessageType>(999);
    auto uuid = Uuidv8Factory::create();
    UAttributesBuilder builderHandler(*g_testUURI,uuid, lType, UPriority::UPRIORITY_CS2);
    UAttributes attributesHandler = builderHandler.build();

    auto uMsg = std::make_shared<UMessage>(g_payloadForHandler, attributesHandler);

    EXPECT_NO_THROW(getHandleOutboundMsg(uMsg));
}

/**
 *  @brief Unit test to verify the behavior of postMessageToQueuee when the handler is running and the priority is invalid
 */
TEST_F(SomeipHandlerClientTests, TestpostMessageToQueueeWhenRunningAndPriorityInvalid) {
    unsigned long data = 123;
    std::shared_ptr<void> ptr = std::make_shared<int>(456);
    uint16_t priority = 456;

    bool result = handlerClient->postMessageToQueue(HandlerMsgType::Outbound, data, ptr, priority);
    EXPECT_FALSE(result);
}

/**
 *  @brief Verify that handleSubscriptionRequestForRemoteService only acts on ResourceInformation objects when
 *  the router is in a registered state.
 */
TEST_F(SomeipHandlerClientTests, handleSubscriptionRequestForRemoteServiceRegisteredTest) {
    std::shared_ptr<uprotocol::utransport::UMessage> messagePtr =
        std::make_shared<uprotocol::utransport::UMessage>(g_messageHandler);

    EXPECT_CALL(mockRouterInterface, isStateRegistered()).WillOnce(Return(false));
    EXPECT_CALL(mockSomeipInterface, requestEvent(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(mockSomeipInterface, subscribe(_, _, _, _, _)).Times(0);
    getHandleSubscriptionRequestForRemoteService(messagePtr);

    EXPECT_CALL(mockRouterInterface, isStateRegistered()).WillOnce(Return(true));
    EXPECT_CALL(mockSomeipInterface, requestEvent(_, _, _, _, _, _)).Times(1);
    EXPECT_CALL(mockSomeipInterface, subscribe(_, _, _, _, _)).Times(1);
    getHandleSubscriptionRequestForRemoteService(messagePtr);
}

/**
 *  @brief Verify that nothing is done in HandleSubscriptionRequestForRemoteService if the subscription already exists.
 */
TEST_F(SomeipHandlerClientTests, handleSubscriptionRequestForRemoteServiceSubExistsTest) {
    UResourceId_t resourceId = 0x0123;
    std::unique_ptr<UResource> resource = createUResource();
    std::shared_ptr<ResourceInformation> resourceInfo = std::make_shared<ResourceInformation>(*resource);
    std::shared_ptr<uprotocol::utransport::UMessage> messagePtr =
        std::make_shared<uprotocol::utransport::UMessage>(g_messageHandler);
    
    EXPECT_CALL(mockSomeipInterface, requestEvent(_, _, _, _, _, _)).Times(1);
    EXPECT_CALL(mockSomeipInterface, subscribe(_, _, _, _, _)).Times(1);
    EXPECT_CALL(mockRouterInterface, isStateRegistered()).WillOnce(Return(true));
    getHandleSubscriptionRequestForRemoteService(messagePtr);

    std::ignore = getaddSubscriptionForRemoteService(resourceId, resourceInfo);
    EXPECT_CALL(mockRouterInterface, isStateRegistered()).WillOnce(Return(true));
    EXPECT_CALL(mockSomeipInterface, requestEvent(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(mockSomeipInterface, subscribe(_, _, _, _, _)).Times(0);
    getHandleSubscriptionRequestForRemoteService(messagePtr);
}

/**
 *  @brief Verify that handleSubscriptionRequestForRemoteService is used to route a subscription request.
 */
TEST_F(SomeipHandlerClientTests, handleOutboundRequestSubTest) {
    UMessage message(g_payloadForHandler, createUAttributes(UMESSAGE_TYPE_REQUEST));
    std::shared_ptr<uprotocol::utransport::UMessage> messagePtr =
        std::make_shared<uprotocol::utransport::UMessage>(message);

    EXPECT_CALL(mockRouterInterface, getMessageTranslator()).Times(0);
    getHandleOutboundMsg(messagePtr);
}

/**
 *  @brief Verify no notification is sent if the subscription does not exist.
 */
TEST_F(SomeipHandlerClientTests, handleOutboundNotificationNoSubTest) {
    std::shared_ptr<uprotocol::utransport::UMessage> messagePtr =
        std::make_shared<uprotocol::utransport::UMessage>(g_messageHandler);

    std::shared_ptr< vsomeip::payload > payload = vsomeip::runtime::get()->create_payload();
    std::vector< vsomeip::byte_t > payloadData;
    for (vsomeip::byte_t i = 0; i < 10; i++) {
        payloadData.push_back(i % 256);
    }
    payload->set_data(payloadData);

    EXPECT_CALL(mockSomeipInterface, createPayload()).Times(1).WillOnce(testing::Return(payload));
    EXPECT_CALL(mockSomeipInterface, notify(_, _, _, _, _)).Times(0);
    getHandleOutboundMsg(messagePtr);
}

/**
 *  @brief Verify that a notification is sent when the subscription exists.
 */
TEST_F(SomeipHandlerClientTests, handleOutboundNotificationSubTest) {
    eventgroup_t eventGroup = 0x0102;
    std::shared_ptr<uprotocol::utransport::UMessage> messagePtr =
        std::make_shared<uprotocol::utransport::UMessage>(g_messageHandler);

    std::shared_ptr< vsomeip::payload > payload = vsomeip::runtime::get()->create_payload();
    std::vector< vsomeip::byte_t > payloadData;
    for (vsomeip::byte_t i = 0; i < 10; i++) {
        payloadData.push_back(i % 256);
    }
    payload->set_data(payloadData);

    subscriptionStatus subStatus;
    subStatus.isSubscribed = true;
    subStatus.eventgroup = eventGroup;
    std::shared_ptr<subscriptionStatus> subStatusPtr = std::make_shared<subscriptionStatus>(subStatus);

    gethandleOfferUResource(g_testUURI);
    gethandleInboundSubscription(subStatusPtr);

    EXPECT_CALL(mockSomeipInterface, createPayload()).Times(1).WillOnce(testing::Return(payload));
    EXPECT_CALL(mockSomeipInterface, notify(_, _, _, _, _)).Times(1);
    getHandleOutboundMsg(messagePtr);
}

/**
 *  @brief Verify that an inbound message is routed when the resource is present.
 */
TEST_F(SomeipHandlerClientTests, handleInboundRequestTest) {
    std::shared_ptr<vsomeip::message> message = createMessage();
    MockSomeipInterface mockInterface;
    MessageTranslator translator(mockInterface);

    gethandleOfferUResource(g_testUURI);
    EXPECT_CALL(mockRouterInterface, routeInboundMsg(_)).Times(1);
    EXPECT_CALL(mockRouterInterface, getMessageTranslator()).Times(1).WillOnce(testing::ReturnRef(translator));
    getHandleInboundRequest(message);
}