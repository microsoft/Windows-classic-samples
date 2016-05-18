//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include <strsafe.h>
#include "PurchaseOrder.wsdl.h"

// Print out rich error info
void PrintError(
    __in HRESULT errorCode, 
    __in_opt WS_ERROR* error)
{
    wprintf(L"Failure: errorCode=0x%lx\n", errorCode);

    if (errorCode == E_INVALIDARG || errorCode == WS_E_INVALID_OPERATION)
    {
        // Correct use of the APIs should never generate these errors
        wprintf(L"The error was due to an invalid use of an API.  This is likely due to a bug in the program.\n");
        DebugBreak();
    }

    HRESULT hr = S_OK;
    if (error != NULL)
    {
        ULONG errorCount;
        hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
        if (FAILED(hr))
        {
            goto Exit;
        }
        for (ULONG i = 0; i < errorCount; i++)
        {
            WS_STRING string;
            hr = WsGetErrorString(error, i, &string);
            if (FAILED(hr))
            {
                goto Exit;
            }
            wprintf(L"%.*s\n", string.length, string.chars);
        }
    }
Exit:
    if (FAILED(hr))
    {
        wprintf(L"Could not get error string (errorCode=0x%lx)\n", hr);
    }
}

HANDLE closeServer = NULL;  


static const WCHAR ExpectedShipDate [] = L"1/1/2006";
static const WCHAR OrderStatusString [] = L"Pending";

static const WS_XML_STRING serviceName = WS_XML_STRING_VALUE("PurchaseOrderService");
static const WS_XML_STRING serviceNamespace = WS_XML_STRING_VALUE("http://example.org");

static const WS_XML_STRING portName = WS_XML_STRING_VALUE("IPurchaseOrder");
static const WS_XML_STRING bindingName = WS_XML_STRING_VALUE("PurchaseOrderBinding");
static const WS_XML_STRING bindingNs = WS_XML_STRING_VALUE("http://example.org");

// The WSDL document used by this example
const static WS_XML_STRING wsdl = WS_XML_STRING_VALUE(
"<wsdl:definitions "
"    xmlns:soap='http://schemas.xmlsoap.org/wsdl/soap/' "
"    xmlns:wsu='http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd' "
"    xmlns:soapenc='http://schemas.xmlsoap.org/soap/encoding/' "
"    xmlns:tns='http://example.org' xmlns:wsa='http://schemas.xmlsoap.org/ws/2004/08/addressing' "
"    xmlns:wsp='http://schemas.xmlsoap.org/ws/2004/09/policy' "
"    xmlns:wsap='http://schemas.xmlsoap.org/ws/2004/08/addressing/policy' "
"    xmlns:xsd='http://www.w3.org/2001/XMLSchema' "
"    xmlns:msc='http://schemas.microsoft.com/ws/2005/12/wsdl/contract' "
"    xmlns:wsaw='http://www.w3.org/2006/05/addressing/wsdl' "
"    xmlns:soap12='http://schemas.xmlsoap.org/wsdl/soap12/' xmlns:wsa10='http://www.w3.org/2005/08/addressing' "
"    xmlns:wsx='http://schemas.xmlsoap.org/ws/2004/09/mex' xmlns:wsdl='http://schemas.xmlsoap.org/wsdl/' "
"    targetNamespace='http://example.org'>"
"  <wsp:Policy wsu:Id='NetTcpBinding_HelloService_policy'>"
"   <wsp:ExactlyOne>"
"      <wsp:All>"
"       <msb:BinaryEncoding xmlns:msb='http://schemas.microsoft.com/ws/06/2004/mspolicy/netbinary1'/>"
"       <wsaw:UsingAddressing/>"
"      </wsp:All>"
"   </wsp:ExactlyOne>"
"  </wsp:Policy>"
"  <wsdl:types>"
"    <xsd:schema targetNamespace='http://example.org/Imports'>"
"      <xsd:import namespace='http://example.org' />"
"    </xsd:schema>"
"  </wsdl:types>"
"  <wsdl:message name='PurchaseOrder'>"
"		<wsdl:part name='parameters' element='tns:PurchaseOrderType'/>"
"  </wsdl:message>"
"  <wsdl:message name='OrderConfirmation'>"
"		<wsdl:part name='parameters' element='tns:OrderConfirmationType'/>"
"  </wsdl:message>"
"  <wsdl:message name='GetOrderStatus'>"
"		<wsdl:part name='parameters' element='tns:GetOrderStatusType'/>"
"  </wsdl:message>"
"  <wsdl:message name='GetOrderStatusResponse'>"
"     <wsdl:part name='parameters' element='tns:GetOrderStatusResponseType'/>"
"  </wsdl:message>"
"  <wsdl:portType name='IPurchaseOrder'>"
"    <wsdl:operation name='Order'>"
"       <wsdl:input message='tns:PurchaseOrder' wsaw:Action='http://example.org/purchaseorder'/>"
"       <wsdl:output message='tns:OrderConfirmation' wsaw:Action='http://example.org/orderconfirmation'/>"
"    </wsdl:operation>"
"    <wsdl:operation name='OrderStatus'>"
"       <wsdl:input message='tns:GetOrderStatus' wsaw:Action='http://example.org/getorderstatus'/>"
"       <wsdl:output message='tns:GetOrderStatusResponse' wsaw:Action='http://example.org/getorderstatusresponse'/>"
"     </wsdl:operation>"
"  </wsdl:portType>"
" <wsdl:binding name='PurchaseOrderBinding' type='tns:IPurchaseOrder'>"
"  <wsp:PolicyReference URI='#NetTcpBinding_HelloService_policy'/>"
"  <soap12:binding transport='http://schemas.microsoft.com/soap/tcp'/>"
"		<wsdl:operation name='Order'>"
"			<soap12:operation soapAction='http://example.org/purchaseorder' style='document'/>"
"			<wsdl:input>"
"				<soap12:body use='literal'/>"
"			</wsdl:input>"
"			<wsdl:output>"
"				<soap12:body use='literal'/>"
"			</wsdl:output>"
"		</wsdl:operation>"
"		<wsdl:operation name='OrderStatus'>"
"			<soap12:operation soapAction='http://example.org/getorderstatus' style='document'/>"
"			<wsdl:input>"
"				<soap12:body use='literal'/>"
"			</wsdl:input>"
"			<wsdl:output>"
"				<soap12:body use='literal'/>"
"			</wsdl:output>"
"		</wsdl:operation>"
"	</wsdl:binding>"
"</wsdl:definitions>"
);

static const WS_XML_STRING xsd = WS_XML_STRING_VALUE(
"<xsd:schema xmlns:tns='http://example.org'"
"elementFormDefault='qualified' "
"targetNamespace='http://example.org' "
"xmlns:xsd='http://www.w3.org/2001/XMLSchema'>"
"<xsd:element name='PurchaseOrderType'>"
"<xsd:complexType>"
"<xsd:sequence>"
"<xsd:element minOccurs='0' name='quantity' type='xsd:int'/>"
"<xsd:element minOccurs='0' name='productName' type='xsd:string'/>"
"</xsd:sequence>"
"</xsd:complexType>"
"</xsd:element>"
"<xsd:element name='OrderConfirmationType'>"
"<xsd:complexType>"
"<xsd:sequence>"
"<xsd:element minOccurs='0' name='orderID' type='xsd:unsignedInt'/>"
"<xsd:element minOccurs='0' name='expectedShipDate' type='xsd:string'/>"
"</xsd:sequence>"
"</xsd:complexType>"
"</xsd:element>"
"<xsd:element name='GetOrderStatusType'>"
"<xsd:complexType>"
"<xsd:sequence>"
"<xsd:element minOccurs='0' name='orderID' type='xsd:unsignedInt'/>"
"</xsd:sequence>"
"</xsd:complexType>"
"</xsd:element>"
"<xsd:element name='GetOrderStatusResponseType'>"
"<xsd:complexType>"
"<xsd:sequence>"
"<xsd:element minOccurs='0' name='orderID' type='xsd:unsignedInt'/>"
"<xsd:element minOccurs='0' name='status' type='xsd:string'/>"
"</xsd:sequence>"
"</xsd:complexType>"
"</xsd:element>"
"</xsd:schema>"
);

static const WS_STRING wsdlDocumentName = WS_STRING_VALUE(L"wsdl");
static const WS_STRING xsdDocumentName = WS_STRING_VALUE(L"xsd");
static const WS_SERVICE_METADATA_DOCUMENT wsdlDocument =
{
    (WS_XML_STRING*)&wsdl,
    (WS_STRING*)&wsdlDocumentName
};

static const WS_SERVICE_METADATA_DOCUMENT xsdDocument =
{
    (WS_XML_STRING*)&xsd,
    (WS_STRING*)&xsdDocumentName
};

static const WS_SERVICE_METADATA_DOCUMENT* metadataDocuments[] =
{
    &wsdlDocument,
    &xsdDocument
};
                
HRESULT CALLBACK PurchaseOrderImpl(
    __in const WS_OPERATION_CONTEXT* context,
    __in int quantity,
    __in_opt __nullterminated WCHAR* productName,
    __out unsigned int* orderID,
    __out_opt __deref __nullterminated WCHAR** expectedShipDate,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext,
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    WS_HEAP* heap = NULL;
    HRESULT hr = S_OK;
    
    wprintf(L"%ld, %s\n", quantity, productName);
    fflush(stdout);
    
    hr = WsGetOperationContextProperty(context, WS_OPERATION_CONTEXT_PROPERTY_HEAP, &heap, sizeof(heap), error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = WsAlloc(heap, sizeof(ExpectedShipDate), (void**)expectedShipDate, error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = StringCbCopyW(*expectedShipDate, sizeof(ExpectedShipDate), ExpectedShipDate);
    if (FAILED(hr))
    {
        return hr;
    }
    
    *orderID = 123;    
    return S_OK;
}

HRESULT CALLBACK GetOrderStatusImpl(
    __in const WS_OPERATION_CONTEXT* context,
    __inout unsigned int* orderID,
    __out_opt __deref __nullterminated WCHAR** status,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext,
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    WS_HEAP* heap = NULL;
    HRESULT hr = S_OK;
    
    *orderID = *orderID;
    
    hr = WsGetOperationContextProperty(context, WS_OPERATION_CONTEXT_PROPERTY_HEAP, &heap, sizeof(heap), error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = WsAlloc(heap, sizeof(OrderStatusString), (void**)status, error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = StringCbCopyW(*status, sizeof(OrderStatusString), OrderStatusString);
    if (FAILED(hr))
    {
        return hr;
    }
    
    return S_OK;
}

ULONG numberOfSession = 0;
HRESULT CALLBACK CloseChannelCallback(
    __in const WS_OPERATION_CONTEXT* context, 
    __in const WS_ASYNC_CONTEXT* asyncContext)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);

    if (InterlockedIncrement(&numberOfSession) == 2)
    {
        SetEvent(closeServer);
    }
    return S_OK;
}

static const PurchaseOrderBindingFunctionTable purchaseOrderFunctions = {PurchaseOrderImpl, GetOrderStatusImpl};

// Method contract for the service
static const WS_SERVICE_CONTRACT purchaseOrderContract = 
{
    &PurchaseOrder_wsdl.contracts.PurchaseOrderBinding, // comes from the generated header.
    NULL, // for not specifying the default contract
    &purchaseOrderFunctions // specified by the user
};


// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_SERVICE_HOST* host = NULL;
    WS_SERVICE_ENDPOINT serviceEndpoint = {0};
    const WS_SERVICE_ENDPOINT* serviceEndpoints[1];
    serviceEndpoints[0] = &serviceEndpoint;
    WS_ERROR* error = NULL;
    WS_SERVICE_PROPERTY serviceProperties[1];
    WS_SERVICE_METADATA serviceMetadata = {0};
    WS_SERVICE_ENDPOINT_PROPERTY serviceEndpointProperties[4];
    WS_SERVICE_PROPERTY_CLOSE_CALLBACK closeCallbackProperty = {CloseChannelCallback};
    WS_SERVICE_ENDPOINT_METADATA endpointMetadata = {0};
    WS_METADATA_EXCHANGE_TYPE metadataExchangeType = WS_METADATA_EXCHANGE_TYPE_MEX;
    
    // Configure Port on the endpoint for Mex
    endpointMetadata.portName = (WS_XML_STRING*)&portName;
    endpointMetadata.bindingName = (WS_XML_STRING*)&bindingName;
    endpointMetadata.bindingNs = (WS_XML_STRING*)&bindingNs;                    
    
    serviceEndpointProperties[0].id = WS_SERVICE_ENDPOINT_PROPERTY_CLOSE_CHANNEL_CALLBACK;
    serviceEndpointProperties[0].value = &closeCallbackProperty;
    serviceEndpointProperties[0].valueSize = sizeof(closeCallbackProperty);
    
    // Specifying Port on the endpoint.
    serviceEndpointProperties[1].id = WS_SERVICE_ENDPOINT_PROPERTY_METADATA;
    serviceEndpointProperties[1].value = &endpointMetadata;
    serviceEndpointProperties[1].valueSize = sizeof(endpointMetadata);
    
    // Marking the endpoint to service WS-MetadataExchnage Requests
    serviceEndpointProperties[2].id = WS_SERVICE_ENDPOINT_PROPERTY_METADATA_EXCHANGE_TYPE;
    serviceEndpointProperties[2].value = &metadataExchangeType;
    serviceEndpointProperties[2].valueSize = sizeof(metadataExchangeType);
    
    WS_STRING mexPrefix = WS_STRING_VALUE(L"mex");
    // Marking the endpoint to service WS-MetadataExchnage Requests
    serviceEndpointProperties[3].id = WS_SERVICE_ENDPOINT_PROPERTY_METADATA_EXCHANGE_URL_SUFFIX;
    serviceEndpointProperties[3].value = &mexPrefix;
    serviceEndpointProperties[3].valueSize = sizeof(mexPrefix);
    
    serviceEndpoint.address.url.chars = L"net.tcp://localhost/example"; // address given as uri
    serviceEndpoint.address.url.length = (ULONG)wcslen(serviceEndpoint.address.url.chars);
    serviceEndpoint.channelBinding = WS_TCP_CHANNEL_BINDING; // channel binding for the endpoint
    serviceEndpoint.channelType = WS_CHANNEL_TYPE_DUPLEX_SESSION; // the channel type
    serviceEndpoint.contract = (WS_SERVICE_CONTRACT*)&purchaseOrderContract;  // the contract
    serviceEndpoint.properties = serviceEndpointProperties;
    serviceEndpoint.propertyCount = WsCountOf(serviceEndpointProperties);
        
    // Specifying WSDL document
    serviceMetadata.documentCount = WsCountOf(metadataDocuments);
    serviceMetadata.documents = (WS_SERVICE_METADATA_DOCUMENT**) metadataDocuments;
    
    // Initializing name of the service
    serviceMetadata.serviceName = (WS_XML_STRING*)&serviceName;
    
    // Note that this should concide be the target namespace of the wsdl document 
    serviceMetadata.serviceNs = (WS_XML_STRING*)&serviceNamespace;
    
    // Specifying metadata document
    serviceProperties[0].id = WS_SERVICE_PROPERTY_METADATA;
    serviceProperties[0].value =  &serviceMetadata;
    serviceProperties[0].valueSize = sizeof(serviceMetadata);
    
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // Create Event object for closing the server
    closeServer = CreateEvent(
        NULL, 
        TRUE, 
        FALSE, 
        NULL);
    if (closeServer == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }   
    // Creating a service host
    hr = WsCreateServiceHost(
        serviceEndpoints, 
        1, 
        serviceProperties, 
        WsCountOf(serviceProperties), 
        &host, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // WsOpenServiceHost to start the listeners in the service host 
    hr = WsOpenServiceHost(
        host, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    WaitForSingleObject(closeServer, INFINITE);
    // Close the service host
    hr = WsCloseServiceHost(host, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (host != NULL)
    {
        WsFreeServiceHost(host);
    }
    
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
    if (closeServer != NULL)
    {
        CloseHandle(closeServer);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}

