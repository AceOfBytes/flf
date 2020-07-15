# FLF (Fast Little Flasher)
This is a vary simple and lean library and executable (the executable is ~28K in size and the lib is ~22K)
to flash devices or copy files fast and easy.

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
