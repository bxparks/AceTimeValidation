# Compare with Java

Compare the [AceTime](https://github.com/bxparks/AceTime) library with the Java
`java.time` package.
Generates the `validation_data.json` file needed by:

* `JavaBasicTest/`
* `JavaExtendedTest/`

## Requirements

You need to install the Java 11 JDK to get the `javac` compiler.

**Ubuntu 18.04, 20.04, 22.04**

```
$ sudo apt install openjdk-11-jdk
```

## Blacklist

The `blacklist.json` file contains zones whose DST offsets do not match AceTime
library or Hinnant date library. I believe these are errors in the java.time
library due to the way it handles negative DST offsets.
