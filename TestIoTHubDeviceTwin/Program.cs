using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;

namespace TestIoTHubDeviceTwin
{
    class Program
    {
        private const string DeviceConnectionString = "<Replace with Azure IoT Hub connection string for your device>";
        static DeviceClient deviceClient;

        // To change this while device is running update the Device Twin with
        // "sensorDataReportIntervalMinutes": 5
        static int sensorDataReportIntervalMinutes = 2;

        static void Main(string[] args)
        {
            // Create the IoT Hub Device Client instance
            deviceClient = DeviceClient.CreateFromConnectionString(DeviceConnectionString, TransportType.Mqtt);

            deviceClient.SetDesiredPropertyUpdateCallback(OnDesiredPropertyChanged, null);

            SynceDeviceWithDeviceTwin().Wait();

            Console.WriteLine("Press enter to exit...");
            Console.ReadLine();
            Console.WriteLine("Exiting...");
        }

        private static async Task SynceDeviceWithDeviceTwin()
        {
            var twin = await deviceClient.GetTwinAsync();
            TwinCollection desiredProperties = twin.Properties.Desired;

            await UpdateDeviceProperties(desiredProperties);
        }

        private static async Task OnDesiredPropertyChanged(TwinCollection desiredProperties, object userContext)
        {
            await UpdateDeviceProperties(desiredProperties);
        }

        private static async Task UpdateDeviceProperties(TwinCollection desiredProperties)
        {
            // Set properties based on Desired Properties (from Device Twin stored in cloud)
            if (desiredProperties.Contains("sensorDataReportIntervalMinutes"))
            {
                sensorDataReportIntervalMinutes = desiredProperties["sensorDataReportIntervalMinutes"];
            }

            // Set Reported Propeeties based on actual device settings
            TwinCollection reportedProperties = new TwinCollection();
            reportedProperties["sensorDataReportIntervalMinutes"] = sensorDataReportIntervalMinutes;

            // Update Device Twin stored in the cloud
            await deviceClient.UpdateReportedPropertiesAsync(reportedProperties);
        }

    }
}
