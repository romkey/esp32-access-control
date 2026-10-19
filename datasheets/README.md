# Datasheets

Manufacturer datasheets for the hardware this project uses, kept here for
reference. **They are not covered by this repository's license**; each
belongs to its manufacturer.

Every datasheet has a license file named after it, minus the extension, with
`-license.txt` appended: `SCD4x.pdf` → `SCD4x-license.txt`. A pre-commit
check fails if one is missing. Template:

```text
<FILE> is copyright <MANUFACTURER> and is not covered by this
repository's license. It is included unmodified, for reference only.
For terms of use and the latest version, see the manufacturer:

Source: <URL the datasheet was downloaded from>
Retrieved: <YYYY-MM-DD>
```

Use the URL the datasheet was actually downloaded from. If a manufacturer's
terms don't allow redistribution, don't commit the PDF; link to it from
docs/ instead.
