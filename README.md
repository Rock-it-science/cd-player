# CD-Player

This project explores using C++ to read information from an attached CD drive.

**Project Goals:**

1. Play audio from a CD loaded into the attached drive
2. Look up metadata for the CD including album artwork, artist name, and track names from MusicBrainz
3. Display album artwork in a graphical interface
4. Enable audio controls via on-screen or physical inputs

## Developer instructions

Conan install:

`conan install .`

cMake setup:

`cmake --preset conan-release`

cMake build:

`cmake --build build/Release`

Run the build file:

`./build/Release/bin/main`
