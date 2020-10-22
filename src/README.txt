README.txt
===============================================================================
                                 Stone - AI for Brood War
===============================================================================

Stone is an AI for Brood War based on the BWAPI library (http://bwapi.github.io/index.html).

It focuses on early attack with the first available units (SCVs)
To compensate for their small number and the low economy, the units rely on their many behaviors.
Multi-agent oriented.

By the way, Stone demonstrates the use of the new BWEM library (http://bwem.sourceforge.net/index.html), which provides fast and robut map analysis.
Stone's SCVs ability to outperform some of their opponents heavily relies the functionality BWEM provides, as the concept of altitude.


-------------------------------------------------------------------------------
Installation:

A DLL is provided that can be used directly.

Stone's source code only consists in standard C++ files.
It uses some of the C++11 features, and possibly some of the C++14 features.
You can compile it at least with Visual Studio 2013 Express, for which a project file is provided.

Except for the BWAPI library, Stone's source code doesn't depend on any other external library (the BWEM library it uses is embedded).

Windows Environment Variables:
- BWAPI_DIR (as suggested in http://webdocs.cs.ualberta.ca/~cdavid/starcraftaicomp/registration.shtml)


-------------------------------------------------------------------------------
License:               MIT/X11

-------------------------------------------------------------------------------
Version:               1.0

-------------------------------------------------------------------------------
Contact:               igor_dimitrijevic@yahoo.fr

-------------------------------------------------------------------------------

