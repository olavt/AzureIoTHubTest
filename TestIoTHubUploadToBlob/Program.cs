using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Azure.Devices.Client;
using System.IO;

namespace TestIoTHubUploadToBlob
{
    class Program
    {
        private const string DeviceConnectionString = "<Replace with Azure IoT Hub connection string for your device>";
        const string sourceFilePath = @"<Path to a local file to upload>";
        static DeviceClient deviceClient;

        static void Main(string[] args)
        {
            string targetFilename = Path.GetFileName(sourceFilePath);

            // Create the IoT Hub Device Client instance
            deviceClient = DeviceClient.CreateFromConnectionString(DeviceConnectionString, TransportType.Mqtt);

            UploadToBlobAsync(sourceFilePath, targetFilename);

            Console.WriteLine("Press enter to exit...");

            Console.ReadLine();
            Console.WriteLine("Exiting...");
        }

        private static async void UploadToBlobAsync(string sourceFilePath, string targetFilename)
        {
            Console.WriteLine($"Uploading file: {sourceFilePath}");
            var watch = System.Diagnostics.Stopwatch.StartNew();

            using (var sourceData = new FileStream(sourceFilePath, FileMode.Open))
            {
                await deviceClient.UploadToBlobAsync(targetFilename, sourceData);
            }

            watch.Stop();
            Console.WriteLine($"Upload of file to Blob completed. Time to upload file: {watch.ElapsedMilliseconds}ms\n");
        }
    }
}
