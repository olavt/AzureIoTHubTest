using Microsoft.Azure.Devices;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestIoTHubReceiveFileUploadNotification
{
    class Program
    {
        static string downloadPath = @"<Replace with a path to a local directory>";
        static string storageConnectionString = "<Replace with the connection string to your Azure Storage Account (Blob)>";
        static string storageContainerName = "<Replace with the container name linked to the IoT Hub";
        static string iotHubConnectionString = "<Replace with a connection string to your Azure IoT Hub>";

        static CloudStorageAccount storageAccount;
        static CloudBlobClient blobClient;
        static ServiceClient serviceClient;
        static void Main(string[] args)
        {
            Console.WriteLine("Receive file upload notifications\n");

            storageAccount = CloudStorageAccount.Parse(storageConnectionString);
            blobClient = storageAccount.CreateCloudBlobClient();

            serviceClient = ServiceClient.CreateFromConnectionString(iotHubConnectionString);
            ReceiveFileUploadNotificationAsync().Wait();
            Console.ReadLine();
        }

        private async static Task ReceiveFileUploadNotificationAsync()
        {
            var notificationReceiver = serviceClient.GetFileNotificationReceiver();

            Console.WriteLine("\nReceiving file upload notification from service");
            while (true)
            {
                var fileUploadNotification = await notificationReceiver.ReceiveAsync();
                if (fileUploadNotification == null) continue;

                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine("Received file upload noticiation: {0}", string.Join(", ", fileUploadNotification.BlobName));

                // Download the blob to the VM disk. The existing tool does not know how to process blobs
                // The processing of the file will be done by an existing tool in another process
                string blobFilename = Path.GetFileName(fileUploadNotification.BlobName);
                string targetPath = $"{downloadPath}\\{blobFilename}";
                await DownloadBlobToFile(fileUploadNotification.BlobName, targetPath);

                Console.WriteLine($"File downloaded to: {targetPath}");
                Console.ResetColor();

                // We are done with the file
                await notificationReceiver.CompleteAsync(fileUploadNotification);
            }
        }

        private async static Task DownloadBlobToFile(string blobName, string targetPath)
        {
            // Retrieve reference to a previously created container.
            CloudBlobContainer container = blobClient.GetContainerReference(storageContainerName);

            // Retrieve reference to the blob
            CloudBlockBlob blockBlob = container.GetBlockBlobReference(blobName);

            await blockBlob.DownloadToFileAsync(targetPath, System.IO.FileMode.Create);
        }

    }
}
