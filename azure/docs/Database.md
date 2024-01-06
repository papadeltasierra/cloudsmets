# Azure Database Design
## Overview
- Use Azure CosmosDB, free-tier
- Use PostgreSQL to Cosmon DB ins column mode

## Database Structure
- Needs to handle at least electricity and gas usage
- Needs to store usage in _energy-units_ over time
- Needs to store usage in _cost-units_ over time
- Needs to store pricing in _cost-per-enery-units_ over time

> Note that SMETS may report _daily total_ figures so post-processing might be required if we want _in this period_ figures.

- Needs to aggregate usage over day, week, month (with/without normalizing to 28 days)
- Need to be able to delete old data to ensure database storage does not grow dramatically over time
- Need to be able to graph the data
  - Graph as instantaneous usage, not totals
  - Graph both _electricity_ or _gas_ or _combined_ (two lines?)
    - Need to show blocks to cover the periods between readings
  - Graph both as _energy-units_ and _cost-units_
  - Graph long-term _cost-per-energy-unit_
  - Need to be able to hover over data and see the raw value
- Nice if could annotate somehow (e.g. "changed supplier")
- Nice if you export data, perhaps as JSON?
- Better if you create custom displays
- Nice if displayed were themselves a table in the database somehow
- Nice if could handle multiple input, so for example graphing an entire family's usage

> Note that Cosmos DB free-tier should be fine for a single household's data.  For cross-family usage, a paid tier might be neccessary.

- Support for cross-family usage would require each reading to carry some sort of _household-identifier_
- Support for cross-family usage would require a table of _household-identifier_ to _Bob's house_ type text.