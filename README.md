# Azure IoT Hub Test code samples

This repository contains a number of small sample code snippets written in C# to test out some useful Azure IoT Hub features.

The samples:
- *TestIoTHubDeviceTwin*: How to use Azure IoT Hub Device Twin functionality from a device
- *TestIoTHubUploadToBlob*: Upload a file from a local device directory to Azure Blob Storage using the Azure IoT Hub DeviceClient.UploadToBlobAsync method
- *TestIoTHubReceiveFileUploadNotification*: How to receive notifications from the cloud when a device uploads a file using using Azure Event Hub
- *TestReceiveFromIoTHub*: How to receive message from the Azure IoT Hub from a .NET Core console application

## How to get started with the samples

Download this repository as a .zip file to your computer, unpack it and then you can open the AzureIoTHubTest.sln with Visual Studio 2017 and run the sample. Note that you need to replace various placeholders in the code with connection strings and secret keys you get from your own Azure resources. 

## Relevant documentation links

[Understand and use device twins in IoT Hub](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-device-twins)

[Upload files from your simulated device to the cloud with IoT Hub](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-csharp-csharp-file-upload)

[Get started receiving messages with the Event Processor Host in .NET Standard](https://docs.microsoft.com/en-us/azure/event-hubs/event-hubs-dotnet-standard-getstarted-receive-eph)