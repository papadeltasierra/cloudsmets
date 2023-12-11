using System;
using System.IO;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using System.Xml;
using System.Data.SqlTypes;
using System.Xml.Linq;

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

        static public SemVer FromString(string semver, bool allowDev)
        {
            int devOffset = semver.IndexOf($"-dev");
            UInt32 major;
            UInt32 minor;
            UInt32 revision;
            UInt32 dev = 0;

            if (devOffset != -1)
            {
                if (! allowDev)
                {
                    return null;
                }
                string devNum = semver[0..^(devOffset + 4)];
                semver = semver[0..^devOffset];
                dev = UInt32.Parse(devNum);
            }

            string[] version = semver.Split('.');

            if (version.Length != 3)
            {
                throw new Exception(String.Format("{0} does not match SemVer syntax.", semver));
            }
            major = UInt32.Parse(version[0]);
            minor = UInt32.Parse(version[1]);
            revision = UInt32.Parse(version[2]);
            return new SemVer(major, minor, revision, dev);
        }


        public void Latest(SemVer candidate)
        {
            if (candidate == null)
            {
                return;
            }

            /**
             * SemVer logic is as described below.  Note that any development version
             * is earlier (not as good) as the non-dev version with same major/minor/revision.
             */
            if ((candidate.major > this.major) ||
                ((candidate.major == this.major) &&
                 ((candidate.minor > this.minor) ||
                  ((candidate.minor == this.minor) &&
                   ((candidate.revision > this.revision) ||
                    ((candidate.revision == this.revision) &&
                     ((candidate.dev == 0) ||
                      ((this.dev != 0) &&
                       (candidate.dev > this.dev)))))))))
            {
                /**
                 * This is a better version so update.
                 */
                this.major = candidate.major;
                this.minor = candidate.minor;
                this.revision = candidate.revision;
                this.dev = candidate.dev;
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


        /**
         * Azure function to offload the listing of available CloudSMETs images and
         * determination of the latest version to Azure.  This is helpful because the
         * default response format from Azure is XML and the ESP32 XML parser (expat)
         * occupies over 80kB.  Doing the work here allows us to save that space
         * in the ESP32 image.
         * 
         * Ref: https://learn.microsoft.com/en-us/troubleshoot/developer/visualstudio/csharp/language-compilers/read-xml-data-from-url
         */
        [FunctionName("Function1")]
        public static async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", Route = null)] HttpRequest req,
            ILogger log)
        {
            Boolean inName = false;
            Boolean allowDev;
            SemVer latestRev = new();

            log.LogInformation("C# HTTP trigger function processed a request.");

            /**
             * Determine whether to allow development versions.
             */
            allowDev = !String.IsNullOrEmpty(req.Query[devParam]);

            XmlTextReader reader = new(containerUrlString);

            while (reader.Read())
            {
                switch (reader.NodeType)
                {
                    case XmlNodeType.Element: // The node is an element.
                        if (reader.Name.Equals(nameElement))
                        {
                            inName = true;
                        }
                        break;

                    case XmlNodeType.Text: //Display the text in each element.
                        if (inName)
                        {
                            latestRev.Latest(SemVer.FromString(reader.Value, allowDev));
                        }
                        break;

                    case XmlNodeType.EndElement: //Display the end of the element.
                        inName = false;
                        break;
                }
            }

            string responseMessage = latestRev.ToString();
            return new OkObjectResult(responseMessage);
        }
    }
}
