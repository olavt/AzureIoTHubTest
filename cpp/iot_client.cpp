// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"

#ifdef MBED_BUILD_TIMESTAMP
#include "certs.h"
#endif // MBED_BUILD_TIMESTAMP

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = "<Replace with connection string for your Azure IoT Hub devicee>";

static int callbackCounter;
static char msgText[2024];
static char propText[2024];
static bool g_continueRunning;
#define MESSAGE_COUNT 50
#define DOWORK_LOOP_NUM     3

#define MAX_BUF 2024

const char * myfifoin = "/tmp/myfifoin";
const char * myfifoout = "/tmp/myfifoout";


typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId;  // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    int* counter = (int*)userContextCallback;
    const char* buffer;
    size_t size;
    MAP_HANDLE mapProperties;
    const char* messageId;
    const char* correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<null>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<null>";
    }

    // Message content
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        (void)printf("unable to retrieve the message data\r\n");
    }
    else
    {
        (void)printf("Received Message [%d]\r\n Message ID: %s\r\n Correlation ID: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n", *counter, messageId, correlationId, (int)size, buffer, (int)size);
        // If we receive the work 'quit' then we stop running
        if (size == (strlen("quit") * sizeof(char)) && memcmp(buffer, "quit", size) == 0)
        {
            g_continueRunning = false;
        }
        
        int fdi = open(myfifoin, O_WRONLY);
        write(fdi,buffer,size);
        close(fdi);
    }

    // Retrieve properties from the message
    mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
                size_t index;

                printf(" Message Properties:\r\n");
                for (index = 0; index < propertyCount; index++)
                {
                    (void)printf("\tKey: %s Value: %s\r\n", keys[index], values[index]);
                }
                (void)printf("\r\n");
            }
        }
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;
    (void)printf("Confirmation[%d] received for message tracking id = %zu with result = %s\r\n", callbackCounter, eventInstance->messageTrackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    /* Some device specific action code goes here... */
    callbackCounter++;
    IoTHubMessage_Destroy(eventInstance->messageHandle);
}

char *basename(char *path)
{
    char *s = strrchr(path, '/');
    if (!s)
        return (path);
    else
        return (s + 1);
}

void iothub_client_sample_mqtt_run(void)
{
    char buffer[MAX_BUF];
    int fdo;
    int count;
    char filename[500];
    
    /* create the FIFO (named pipe) */
    mkfifo(myfifoin, 0666);
    mkfifo(myfifoout, 0666);

    /* open, read, and display the message from the FIFO */
    fdo = open(myfifoout, O_RDONLY);
    
    int flags = fcntl(fdo, F_GETFL, 0);
    if(fcntl(fdo, F_SETFL, flags | O_NONBLOCK));
    
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

    EVENT_INSTANCE messages[MESSAGE_COUNT];

    g_continueRunning = true;
    srand((unsigned int)time(NULL));
//    double avgWindSpeed = 10.0;
    
    callbackCounter = 0;
    int receiveContext = 0;

    if (platform_init() != 0)
    {
        (void)printf("Failed to initialize the platform.\r\n");
    }
    else
    {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) == NULL)
        {
            (void)printf("ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            bool traceOn = true;
            IoTHubClient_LL_SetOption(iotHubClientHandle, "logtrace", &traceOn);

#ifdef MBED_BUILD_TIMESTAMP
            // For mbed add the certificate information
            if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
            {
                printf("failure to set option \"TrustedCerts\"\r\n");
            }
#endif // MBED_BUILD_TIMESTAMP

            /* Setting Message call back, so we can receive Commands. */
            if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
            {
                (void)printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
            }
            else
            {
                (void)printf("IoTHubClient_LL_SetMessageCallback...successful.\r\n");
                
                /* Now that we are ready to receive commands, let's send some messages */
                size_t iterator = 0;
                do
                {
                    // Read real data from sensor system....
                    count = read(fdo, buffer, MAX_BUF);
                    if(count < 0 && errno == EAGAIN) {
                        // If this condition passes, there is no data to be read
                    }
                    else if(count > 0) {
                        float depthraw=0;
                        float depth=0;
                        
                        fprintf(stderr,"Received: %d Bytes\n", count);
                        fprintf(stderr,"Data: %s\n", buffer);
                        
                        // If file to be sent...
                        if(buffer[0] == '#')
                        {
                            fprintf(stderr,"Sending File...\n");
                            
                            sscanf(buffer,"#%s",filename);
                            fprintf(stderr,"File: %s",filename);
                            
                            // Upload a file
                            FILE *f = fopen(filename, "rb");
                            if(f > 0){
                                fprintf(stderr,"File Opened OK\n");
                                fseek(f, 0, SEEK_END);
                                size_t size = ftell(f);
                                unsigned char* s = malloc(size);
                                rewind(f);
                                int bytesRead = fread(s, sizeof(char), size, f);
                                
                                char *file= basename(filename);
                                
                                fprintf(stderr,"File: %s",file);
                                int status = IoTHubClient_LL_UploadToBlob(iotHubClientHandle, file, s, size);
                                
                                free(s);
                                fprintf(stderr,"Size: %d   Status:  %d\n",bytesRead,status);
                            }
                            else{
                                fprintf(stderr,"Error Opening file...\n");
                            }
                            
                        }
                        else{  // If data....
                            sscanf(buffer,"depthraw: %f depth: %f",&depthraw, &depth);
                            
                            sprintf_s(msgText, sizeof(msgText),"[{\"assetid\": \"BerthMonitoring_#001\",\"properties\": [{\"property\": \"Depthraw\",\"doublevalue\":%f,\"stringvalue\": \"stringvalue1\"},{\"property\": \"Depth\",\"doublevalue\": %f,\"stringvalue\": \"stringvalue1\"}],\"timezone\": \"UTC\",\"timestamp\": \"2016-10-30 23:00:00\"}]",depthraw,depth);
                            
                            
                            if ((messages[0].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL)
                            {
                                (void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
                            }
                            else
                            {
                                messages[0].messageTrackingId = iterator;
                                MAP_HANDLE propMap = IoTHubMessage_Properties(messages[0].messageHandle);
                                (void)sprintf_s(propText, sizeof(propText), "PropMsg_%zu", iterator);
                                if (Map_AddOrUpdate(propMap, "PropName", propText) != MAP_OK)
                                {
                                    (void)printf("ERROR: Map_AddOrUpdate Failed!\r\n");
                                }
                                
                                if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages[0].messageHandle, SendConfirmationCallback, &messages[0]) != IOTHUB_CLIENT_OK)
                                {
                                    (void)printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
                                }
                                else
                                {
                                    (void)printf("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", (int)iterator);
                                }
                            }
                            iterator++;
                        }
                    }
                    else {
                        // OK but no data...
                    }
                    
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(10);

                } while (g_continueRunning);

                (void)printf("iothub_client_sample_mqtt has gotten quit message, call DoWork %d more time to complete final sending...\r\n", DOWORK_LOOP_NUM);
                size_t index = 0;
                for (index = 0; index < DOWORK_LOOP_NUM; index++)
                {
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(1);
                }
            }
            IoTHubClient_LL_Destroy(iotHubClientHandle);
        }
        platform_deinit();
    }
    close(fdo);
}

int main(void)
{
    iothub_client_sample_mqtt_run();
    return 0;
}
