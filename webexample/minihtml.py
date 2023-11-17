"""Complete minification of HTML files."""
import os
import sys
import re

RGX_JS = re.compile(r"""(\.js["'])""")
RGX_CSS = re.compile(r"""(\.css["'])""")
RGX_HTML0 = re.compile(r"""\.htm(["'])""")
RGX_HTML1 = re.compile(r">\s+<")
RGX_HTML2 = re.compile(r">\s+")
RGX_HTML3 = re.compile(r"\s+<")
RGX_HTMLFILE = re.compile(r".*\.html$")

for rootdir, directories, filenames in os.walk(sys.argv[1]):
    for filename in filenames:
        if RGX_HTMLFILE.match(filename):
            fullpath = os.path.join(rootdir, filename)
            print("Processing %s..." % fullpath)
            with open(fullpath, "r") as source:
                html = source.read()

            # Add the "-min" to the name of JS files
            html = RGX_JS.sub(r"-min\1", html)
            # Add the "-min" to the name of CSS files
            html = RGX_CSS.sub(r"-min\1", html)
            # Rename "".htm" files to "".html".
            html = RGX_HTML0.sub(r".html\1", html)
            # Remove space betwen HTML items.
            html = RGX_HTML1.sub(r"><", html)
            # Remove leading space in strings.
            html = RGX_HTML2.sub(r">", html)
            # Remove trailing space from stings.
            html = RGX_HTML3.sub(r"<", html)

            with open(fullpath, "w") as target:
                target.write(html)
