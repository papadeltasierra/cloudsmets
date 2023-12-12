using System;
using System.IO;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using System.Xml;
using System.Text.RegularExpressions;

namespace LastestRevision
{

    /**
     * Minimal SemVer implementation.
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

    public static class Function1
    {
        private static readonly string containerUrlString = Environment.GetEnvironmentVariable("containerUrl");
        private static readonly string nameElement = $"Name";
        private static readonly string devParam = $"dev";
        private static readonly Regex version = new (@"^(\d+)\.(\d+)\.(\d+)(?:-dev([1-9]\d*))?/$");

        /**
         * Azure function to offload the listing of available CloudSMETs images and
         * determination of the latest version to Azure.  This is helpful because the
         * default response format from Azure is XML and the ESP32 XML parser (expat)
         * occupies over 80kB.  Doing the work here allows us to save that space
         * in the ESP32 image.
         * 
         * Ref: https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlreader?view=net-8.0
         */
        [FunctionName("Function1")]
        public static async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", Route = null)] HttpRequest req,
            ILogger log)
        {
            Boolean inName = false;
            Boolean allowDev;
            SemVer latestRev = new();

            /**
             * Determine whether to allow development versions.
             */
            allowDev = !String.IsNullOrEmpty(req.Query[devParam]);

            XmlReaderSettings settings = new() {
                Async = true
            };

            using (XmlReader reader = XmlReader.Create(containerUrlString, settings))
            {
                while (await reader.ReadAsync())
                {
                    switch (reader.NodeType)
                    {
                        case XmlNodeType.Element:
                            if (reader.Name.Equals(nameElement))
                            {
                                inName = true;
                            }
                            break;

                        case XmlNodeType.Text:
                            if (inName)
                            {
                                string field = await reader.GetValueAsync();
                                Match match = version.Match(field);
                                if (match.Success)
                                {
                                    latestRev.Latest(UInt32.Parse(match.Groups[1].Value),
                                                     UInt32.Parse(match.Groups[2].Value),
                                                     UInt32.Parse(match.Groups[3].Value),
                                                     match.Groups[4].Success ? UInt32.Parse(match.Groups[4].Value) : 0,
                                                     allowDev);
                                }
                            }
                            break;

                        case XmlNodeType.EndElement:
                            inName = false;
                            break;

                        default:
                            break;
                    }
                }
            }

            string responseMessage = latestRev.ToString();
            return new OkObjectResult(responseMessage);
        }
    }
}
