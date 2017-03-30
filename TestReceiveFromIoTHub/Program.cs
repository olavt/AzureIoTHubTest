using System;
using System.Threading.Tasks;

using Microsoft.Azure.EventHubs;
using Microsoft.Azure.EventHubs.Processor;

namespace TestReceiveFromIoTHub
{
    class Program
    {
        // "Endpoint=sb://<Your namespace>.servicebus.windows.net/;SharedAccessKeyName=<Your name>;SharedAccessKey=<Your key>";
        private const string EhConnectionString = "<Replace with the Event Hub connection string for your IoT Hub on the above format>";
        private const string EhEntityPath = "<Get this from the Event Hub for your IoT Hub>";

        private const string StorageContainerName = "eventreader";
        private static readonly string StorageConnectionString = "<Replace with the connection string to your Azure Storage Account>";

        public static void Main(string[] args)
        {
            MainAsync(args).Wait();
        }

        private static async Task MainAsync(string[] args)
        {
            Console.WriteLine("Registering EventProcessor...");

            var eventProcessorHost = new EventProcessorHost(
                EhEntityPath,
                PartitionReceiver.DefaultConsumerGroupName,
                EhConnectionString,
                StorageConnectionString,
                StorageContainerName);

            // Registers the Event Processor Host and starts receiving messages
            await eventProcessorHost.RegisterEventProcessorAsync<SimpleEventProcessor>();

            Console.WriteLine("Receiving. Press ENTER to stop worker.");
            Console.ReadLine();

            // Disposes of the Event Processor Host
            await eventProcessorHost.UnregisterEventProcessorAsync();
        }

    }
}