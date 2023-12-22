using Azure.Messaging.EventHubs;
using Microsoft.Azure.WebJobs;
using System;
using Microsoft.Extensions.Logging;

namespace mqttreceiver
{
    public class MQTTReceiver
    {
        [FunctionName("mqttreceiver")]
        public void Run([EventHubTrigger("CloudSMETS", Connection = "EventHubConnection")]EventData message, ILogger log)
        {
            log.LogInformation($"C# IoT Hub trigger function processed a message: {BitConverter.ToString(message.EventBody.ToArray()).Replace("-", "")}");
        }
    }
}