/*
 * Copyright (c) 2015-2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Patent rights are not granted under this agreement. Patent rights are
 *       available under FRAND terms.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX or PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @author Kevin Fox, Palo Alto Research Center (Xerox PARC)
 * @copyright 2015-2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC).  All rights reserved.
 */
#include <config.h>
#include "../athena_TransportLinkModuleETH.c"
#include <LongBow/unit-test.h>
#include <stdio.h>

#include <net/if.h>
#ifndef __linux__
#include <net/if_dl.h>
#endif
#include <ifaddrs.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Network.h>
#include <ccnx/forwarder/athena/athena.h>
#include <ccnx/forwarder/athena/athena_TransportLinkAdapter.h>

#define DEVICE "lo"

LONGBOW_TEST_RUNNER(athena_TransportLinkModuleETH)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

LONGBOW_TEST_RUNNER_SETUP(athena_TransportLinkModuleETH)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(athena_TransportLinkModuleETH)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, athenaTransportLinkModuleETH_OpenClose);
    LONGBOW_RUN_TEST_CASE(Global, athenaTransportLinkModuleETH_SendReceive);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

void
_removeLink(void *context, PARCBitVector *parcBitVector)
{
    assertNull(context, "_removeLink called with a non null argument");
    //AthenaTransportLinkAdapter *athenaTransportLinkAdapter = (AthenaTransportLinkAdapter *) context;
}

static char *
_getInterfaceByName(void)
{
    // On Linux we can (must?) use the local loopback interface for testing
#ifdef __linux__
    return strdup("lo");
#endif

    // Lookup the MAC address of an interface that is up, then ask for it.  Don't use loopback.
    struct ifaddrs *ifaddr;
    int failure = getifaddrs(&ifaddr);
    assertFalse(failure, "Error getifaddrs: (%d) %s", errno, strerror(errno));

    char *ifname = NULL;

    struct ifaddrs *next;
    for (next = ifaddr; next != NULL && ifname == NULL; next = next->ifa_next) {
        if ((next->ifa_addr == NULL) || ((next->ifa_flags & IFF_UP) == 0)) {
            continue;
        }

        if (next->ifa_flags & IFF_LOOPBACK) {
            continue;
        }

        if (next->ifa_addr->sa_family == AF_INET) {
            ifname = strdup(next->ifa_name);
        }
    }
    freeifaddrs(ifaddr);
    return ifname;
}

LONGBOW_TEST_CASE(Global, athenaTransportLinkModuleETH_OpenClose)
{
    PARCURI *connectionURI;
    const char *result;
    char linkSpecificationURI[MAXPATHLEN];

    const char *device = _getInterfaceByName();
    assertNotNull(device, "Could not find an available ethernet device");

    AthenaTransportLinkAdapter *athenaTransportLinkAdapter = athenaTransportLinkAdapter_Create(_removeLink, NULL);
    assertNotNull(athenaTransportLinkAdapter, "athenaTransportLinkAdapter_Create returned NULL");

    athenaTransportLinkAdapter_SetLogLevel(athenaTransportLinkAdapter, PARCLogLevel_Debug);

    sprintf(linkSpecificationURI, "eth://%s/name=", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open failed to detect bad name argument");
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth://%s/local=", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open failed to detect bad local argument");
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth://%s/src=", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open failed to detect bad source argument");
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth:///name=ETH_1");
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open failed to detect bad address specification");
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth://%s/Listene/name=ETH_1", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open failed to detect bad argument");
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth://%s/Listener/nameo=", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open failed to detect bad name specification");
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth://%s/Listener/name=", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open failed to detect bad name specification");
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth://%s/Listener/name=ETH_1", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);

    // If we can't open a device (i.e. we're not root), tests can not continue.
    if ((result == NULL) && (errno == EBADF)) {
        parcURI_Release(&connectionURI);
        athenaTransportLinkAdapter_Destroy(&athenaTransportLinkAdapter);
        return;
    }
    assertTrue(result != NULL, "athenaTransportLinkAdapter_Open failed (%s)", strerror(errno));
    parcURI_Release(&connectionURI);

    sprintf(linkSpecificationURI, "eth://%s/Listener/name=ETH_1", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result == NULL, "athenaTransportLinkAdapter_Open succeeded in opening a duplicate link");
    parcURI_Release(&connectionURI);

    int closeResult = athenaTransportLinkAdapter_CloseByName(athenaTransportLinkAdapter, "ETH_1");
    assertTrue(closeResult == 0, "athenaTransportLinkAdapter_CloseByName failed (%s)", strerror(errno));

    athenaTransportLinkAdapter_Destroy(&athenaTransportLinkAdapter);
}

LONGBOW_TEST_CASE(Global, athenaTransportLinkModuleETH_SendReceive)
{
    PARCURI *connectionURI;
    char linkSpecificationURI[MAXPATHLEN];
    char deviceMAC[NI_MAXHOST];
    const char *result;

    AthenaTransportLinkAdapter *athenaTransportLinkAdapter = athenaTransportLinkAdapter_Create(_removeLink, NULL);
    assertNotNull(athenaTransportLinkAdapter, "athenaTransportLinkAdapter_Create returned NULL");

    const char *device = _getInterfaceByName();
    assertNotNull(device, "Could not find an available ethernet device");

    struct ether_addr myAddress;
    athenaEthernet_GetInterfaceMAC(device, &myAddress);
    sprintf(deviceMAC, "%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx:%2.2hhx",
            myAddress.ether_addr_octet[0], myAddress.ether_addr_octet[1],
            myAddress.ether_addr_octet[2], myAddress.ether_addr_octet[3],
            myAddress.ether_addr_octet[4], myAddress.ether_addr_octet[5]);

    sprintf(linkSpecificationURI, "eth://%s/Listener/name=ETHListener", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);

    // If we can't open a device (i.e. we're not root), the test can not continue.
    if ((result == NULL) && (errno == EBADF)) {
        parcURI_Release(&connectionURI);
        athenaTransportLinkAdapter_Destroy(&athenaTransportLinkAdapter);
        return;
    }
    assertTrue(result != NULL, "athenaTransportLinkAdapter_Open failed (%s)", strerror(errno));
    parcURI_Release(&connectionURI);

    // Open a link we can send messages on
    //sprintf(linkSpecificationURI, "eth://%s:%s/name=ETH_1", device, deviceMAC);
    sprintf(linkSpecificationURI, "eth://%s/name=ETH_1", device);
    connectionURI = parcURI_Parse(linkSpecificationURI);
    result = athenaTransportLinkAdapter_Open(athenaTransportLinkAdapter, connectionURI);
    assertTrue(result != NULL, "athenaTransportLinkAdapter_Open failed (%s)", strerror(errno));
    parcURI_Release(&connectionURI);
    free((void *) device);

    // Enable debug logging after all instances are open
    athenaTransportLinkAdapter_SetLogLevel(athenaTransportLinkAdapter, PARCLogLevel_Debug);

    // Construct an interest
    CCNxName *name = ccnxName_CreateFromURI("lci:/foo/bar");
    CCNxMetaMessage *ccnxMetaMessage = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    PARCBitVector *sendVector = parcBitVector_Create();

    // Send the interest out on the link, this message will also be received by ourself
    // since we're sending it to our own MAC destination.
    int linkId = athenaTransportLinkAdapter_LinkNameToId(athenaTransportLinkAdapter, "ETH_1");
    parcBitVector_Set(sendVector, linkId);

    athena_EncodeMessage(ccnxMetaMessage);
    PARCBitVector *resultVector;
    resultVector = athenaTransportLinkAdapter_Send(athenaTransportLinkAdapter, ccnxMetaMessage, sendVector);
    assertNotNull(resultVector, "athenaTransportLinkAdapter_Send failed");
    assertTrue(parcBitVector_NumberOfBitsSet(resultVector) != 0, "athenaTransportLinkAdapter_Send failed");
    parcBitVector_Release(&resultVector);

    // Send the message a second time
    resultVector = athenaTransportLinkAdapter_Send(athenaTransportLinkAdapter, ccnxMetaMessage, sendVector);
    assertNotNull(resultVector, "athenaTransportLinkAdapter_Send failed");
    assertTrue(parcBitVector_NumberOfBitsSet(resultVector) != 0, "athenaTransportLinkAdapter_Send failed");
    parcBitVector_Release(&resultVector);
    parcBitVector_Release(&sendVector);

    ccnxMetaMessage_Release(&ccnxMetaMessage);

    // Allow a context switch for the sends to complete
    usleep(1000);

    ccnxMetaMessage = athenaTransportLinkAdapter_Receive(athenaTransportLinkAdapter, &resultVector, 0);
    assertNotNull(resultVector, "athenaTransportLinkAdapter_Receive failed");
    assertTrue(parcBitVector_NumberOfBitsSet(resultVector) == 1, "athenaTransportLinkAdapter_Receive return message with more than one ingress link");
    assertNotNull(ccnxMetaMessage, "athenaTransportLinkAdapter_Receive failed to provide message");
    parcBitVector_Release(&resultVector);
    ccnxMetaMessage_Release(&ccnxMetaMessage);

    // Receive the duplicate
    ccnxMetaMessage = athenaTransportLinkAdapter_Receive(athenaTransportLinkAdapter, &resultVector, 0);
    parcBitVector_Release(&resultVector);
    ccnxMetaMessage_Release(&ccnxMetaMessage);

    // Receive the second message
    ccnxMetaMessage = athenaTransportLinkAdapter_Receive(athenaTransportLinkAdapter, &resultVector, 0);
    parcBitVector_Release(&resultVector);
    ccnxMetaMessage_Release(&ccnxMetaMessage);

    // Receive the second duplicate
    ccnxMetaMessage = athenaTransportLinkAdapter_Receive(athenaTransportLinkAdapter, &resultVector, 0);
    parcBitVector_Release(&resultVector);
    ccnxMetaMessage_Release(&ccnxMetaMessage);

    // Close one end of the connection
    int closeResult = athenaTransportLinkAdapter_CloseByName(athenaTransportLinkAdapter, "ETHListener");
    assertTrue(closeResult == 0, "athenaTransportLinkAdapter_CloseByName failed (%s)", strerror(errno));

    closeResult = athenaTransportLinkAdapter_CloseByName(athenaTransportLinkAdapter, "ETH_1");
    assertTrue(closeResult == 0, "athenaTransportLinkAdapter_CloseByName failed (%s)", strerror(errno));

    athenaTransportLinkAdapter_Destroy(&athenaTransportLinkAdapter);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(athena_TransportLinkModuleETH);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
