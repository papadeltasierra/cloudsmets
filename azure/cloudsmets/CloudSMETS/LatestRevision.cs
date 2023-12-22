using System;
using System.Threading.Tasks;
using Azure;
using Azure.Storage.Blobs;
using Azure.Storage.Blobs.Models;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using System.Text.RegularExpressions;

/**
 * TODO:
 * - Add documentation in either ESP32 or Azure format.
 * - DONE: Can we add metrics to spot when fails?
 * - Where are the metrics?
 * - Do we need more logging?
 * - Does CI prevent bad SemVer?
 *   - We should log error when we encounter one "just in case"
 *   - Maybe throw error metric too!  
 * - Need to add unit testing and integration testing
 */

namespace CloudSMETS
{
    /**
     * Minimal SemVer implementation.
     * Supports just syntaxes:
     * - 1.2.3 or
     * - 4.5.6-dev7.
     */
    public class SemVer
    {
        private ulong major;
        private ulong minor;
        private ulong revision;
        private ulong dev;

        public SemVer(ulong major = 0, ulong minor = 0, ulong revision = 0, ulong dev = 0)
        {
            this.major = major;
            this.minor = minor;
            this.revision = revision;
            this.dev = dev;
        }

        public void Latest(UInt32 major, UInt32 minor, UInt32 revision, UInt32 dev, bool allowDev)
        {
            /**
             * SemVer logic is as described below.  Note that any development version
             * is earlier (not as good) as the non-dev version with same major/minor/revision.
             */
            if ((major > this.major) ||
                ((major == this.major) &&
                 ((minor > this.minor) ||
                  ((minor == this.minor) &&
                   ((revision > this.revision) ||
                    ((revision == this.revision) &&
                     ((dev == 0) ||
                      ((this.dev != 0) &&
                       (allowDev) &&
                       (dev > this.dev)))))))))
            {
                /**
                 * This is a better version so update.
                 */
                this.major = major;
                this.minor = minor;
                this.revision = revision;
                this.dev = dev;
            }
        }

        public override String ToString()
        {
            if (this.dev == 0)
            {
                /**
                 * Not a development version.
                 */
                return String.Format("{0}.{1}.{2}", this.major, this.minor, this.revision);
            }
            else
            {
                return String.Format("{0}.{1}.{2}-dev{3}", this.major, this.minor, this.revision, this.dev);
            }
        }
    }

    public static class LatestRevision
    {
        private static readonly string devParam = $"dev";
        private static readonly Regex version = new(@"^(\d+)\.(\d+)\.(\d+)(?:-dev([1-9]\d*))?/$");
        private static readonly string[] connectionStrings = {
                Environment.GetEnvironmentVariable("AZURE_STORAGE_CONNECTION_STRING_1"),
                Environment.GetEnvironmentVariable("AZURE_STORAGE_CONNECTION_STRING_2")
            };
        private static readonly string AZURE_STORAGE_CONTAINER_NAME = Environment.GetEnvironmentVariable("AZURE_STORAGE_CONTAINER_NAME");
        private static int connectionStringIndex = 0;

        /**
         * Azure function to offload the listing of available CloudSMETs images and
         * determination of the latest version to Azure.
         * 
         * Note that throwing an Exception causes the function to return a 500 class response.
         */
        [FunctionName("LatestRevision")]
        public static async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", Route = null)] HttpRequest req,
            ILogger log)
        {
            // Boolean inName = false;
            Boolean allowDev;
            SemVer latestRev = new();

            /**
             * Determine whether to allow development versions.
             */
            allowDev = !String.IsNullOrEmpty(req.Query[devParam]);

            /**
             * By always trying "the other" connection string, we will detect errors if 
             * either of them fail.
             */
            int currentConnectionString = (1 - connectionStringIndex);
            int connectionStringAttempts = 2;

            /**
             * Connect to the Azure Storage container.
             */
            BlobServiceClient blobServiceClient;
            BlobContainerClient blobContainerClient = null;

            while (connectionStringAttempts > 0)
            {
                try
                {
                    blobServiceClient = new BlobServiceClient(connectionStrings[currentConnectionString]);
                    blobContainerClient = blobServiceClient.GetBlobContainerClient(AZURE_STORAGE_CONTAINER_NAME);
                    break;
                }
                catch
                {
                    log.LogError(String.Format("Connection string {0} not working", currentConnectionString + 1));
                }
                connectionStringAttempts--;
                currentConnectionString = (1 - currentConnectionString);
            }

            if (connectionStringAttempts == 0)
            {
                log.LogError($"No connection strings working");
                throw new Exception("Internal error");
            }

            /**
             * Store current connection string; next time we start from the other one.
             */
            connectionStringIndex = currentConnectionString;

            /*
             * List the (subdirectory) prefixes in the container
             */
            try
            {
                AsyncPageable<BlobHierarchyItem> blobHierarchyItems = 
                    blobContainerClient.GetBlobsByHierarchyAsync(delimiter: $"/");
                await foreach (BlobHierarchyItem blobHierarchyItem in blobHierarchyItems)
                {
                    Match match = version.Match(blobHierarchyItem.Prefix);
                    if (match.Success)
                    {
                        latestRev.Latest(UInt32.Parse(match.Groups[1].Value),
                                         UInt32.Parse(match.Groups[2].Value),
                                         UInt32.Parse(match.Groups[3].Value),
                                         match.Groups[4].Success ? UInt32.Parse(match.Groups[4].Value) : 0,
                                         allowDev);
                    }
                }
            }
            catch (Exception ex)
            {
                log.LogError(
                    exception: ex,
                    message: $"Failure querying the revisions from the container.");
                throw new Exception("Internal error");
            }

            string responseMessage = latestRev.ToString();
            return new OkObjectResult(responseMessage);
        }
    }
}
