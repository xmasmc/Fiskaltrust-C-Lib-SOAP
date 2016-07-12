call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat"

cl test_lib.c Release_LIB\deplib_test1.lib webservices.lib msvcrt.lib libcmt.lib

@rem cl -c /Fotest_lib.obj test_lib.c 
@rem link  /SUBSYSTEM:console /ENTRY:mainCRTStartup /OUT:"test_lib.exe" test_lib.obj Release_LIB\cppConsoleApplicationSOAP-Test1.lib ^
@rem webservices.lib msvcrt.lib libcmt.lib

pause

