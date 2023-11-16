# Webserver Files
## Overview
- *.html are HTML pages
- *.css are the style sheets
- *.js are Javascript procedures
- *.json are examlpe JSON data

## Minify
We could use the Python `css-html-js-minify` package to minify the data, which approximately halves the size of the files but...

- Need to change references to `*.css` to be `*.min.css`
- Need to change references to `*.js` to be `*.min.js`
- HTML files can be further compacted by removing spaces between HTML elements e.g. `>  <` to `><`.

### Development Process
- Work with expanded files for most times
- Minify at the end of the process
- Post-process HTML files to perform:
    - The name changes for `*.css`
    - The name changes for `*.js`
    - The extra compaction.

We should be able to use very simple pattern changes for all of these.