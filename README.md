# Lib microFlash and the ufl utility
This is a very simple and lean library and cli to flash devices or just copy bytes between files very fast
, the cli is ~28K in size (with the lib statically linked) and the lib is ~22K.

## The cli
The cli was designed mainly to allow use of the lib in languages other than C without bindings primarily.

# Support and API stability
The versioning scheme follows semver, pre v1.0.0 series have support until the next minor bump in the series, revisions
shall not bring breaking API changes any such changes will incur a minor bump. After v1.0.0 series, revisions and minor
version bumps shall not bring API breaking changes those are reserved to major version bumps. At first only the current
major version is supported.

# OS support
This software tries to stick as much as possible to posix APIs, so it should work on most POSIX compliant OSs
but I can only guarantee it works as intended on linux, maybe BSDs.
MacOS is right now out of scope mainly because I lack a mac for testing (if you have a mac and can test and share the
results I would be grateful).
Windows is experimental at best out of scope at worst.

# License
BSD-3-Clause
