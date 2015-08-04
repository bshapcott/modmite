modmite is pre-release and in active development.

The build process requires several manual steps.

Development in performed on Windows Vista using Visual Studio C++ 2008 Express.

Some trial builds have been performed on CentOS Linux, because eventually this project will be cross-platform.

# Details #

Build steps:
  * Get Microsoft Visual Studio C++ 2008 and install.
    * http://www.microsoft.com/exPress/download/
  * Get doxygen and install.
    * http://www.stack.nl/~dimitri/doxygen/
  * Get Apache 2.2.
    * If you get a source distribution, build and install.
    * The binary install does include the necessary libraries and headers, so it is not necessary to get the source distribution.
    * C:\Apache22 is assumed as the install folder for the remainder of these instructions.
    * http://httpd.apache.org/
  * Checkout modmite with subversion to a local folder, such as C:\Project.
    * I.E. C:\Project\mite.sln will exist after the checkout.
    * C:\Project is assumed as the checkout folder for the remainder of these instructions.
  * Get yajl.  Unzip to C:\Project\thirdparty\yajl.
    * I.E. C:\Project\thirdparty\yajl\BUILDING.win32 will exist after unzipping.
    * Follow these modified instructions, rather than BUILDING.win32:
      * mkdir C:\Project\thirdparty\yajl\build.
      * Open the cmake-gui.
        * Set "Where is the source code?" to "C:/Project/thirdparty/yajl".
        * Set "Where to build the binaries?" to "C:/Project/thirdparty/yajl/build".
        * Hit the "Configure" button.
        * Select "Visual Studio 9 2008" as the compiler environment to use.
        * cmake-gui will find doxygen automatically.
        * Hit the "Generate" button.
        * The "devenv YetAnotherJSONParser.sln" step and those following do not need to be followed.  mite.sln will build the required yajl project.
  * Get sqlite almalgamation.  Unzip to C:\Project\thirdparty\sqlite.
    * sqlite-almalgamation distribution does not contain Visual Studion project files or the export definitions (.def) file.  These are checked out from modmite and the overlaid with the source files from sqlite-almalgamation.
    * mite.sln will build sqlite.
  * Open C:\Project\mite\mite.sln.
    * Fixup the yajl project (right-click yajl project in Solution Explorer and pick Properties from the pop-up menu).
      * yajl Property Pages->Linker->Input
        * Module Definition File = $(SolutionDir)\thirdparty\$(ProjectName)\$(TargetName).def
        * This is a pain, but setting WIN32, YAJL\_SHARED and YAJL\_BUILD causes compile errors.
      * yajl Property Pages->Linker->Command Line
        * In the "Additional Options:" panel, remove "/PDB:NONE".
      * yajl Property Pages->Custom Build Step
        * Command Line:
```
@echo on
copy $(TargetPath)  \Apache22\bin\
copy $(TargetDir)\$(TargetName).pdb  \Apache22\bin\
@echo off
```
        * Description = "Copying .dll and .pdb to installation directory . . ."
        * Outputs:
```
\Apache22\bin\$(TargetFileName)
\Apache22\bin\$(TargetName).pdb
```
    * Build the DEBUG project.
      * I haven't checked all the settings in the RELEASE project yet.
    * The sql and htdocs projects are not built automatically.  Build them separately.
      * The sql project creates a sample database called "game.db" that can be used to experiment with the code.
      * The htdocs project copies a custom httpd.conf file to C:\Apache22\conf.
        * The customer httpd.conf points back to C:\Project\htdocs as the httpd server's document root.
        * It also registers the modmite mod, and the required yajl and sqlite dynamic libraries.
  * Debug->Start Debugging (or function key F5).
    * Runs the httpd server with the -X option, which supresses forking child processes, making the server easier to debug.

The build process will become more automated going forward.  There is already an  [issue](http://code.google.com/p/modmite/issues/detail?id=39) for this.