set OUTPUTFILE=TaskInstanceViewMock5.cpp
set HEADERFILE=TaskInstanceView.h
rem set QTDIR=C:\Qt\4.8.5
set QTDIR=I:\Programs\Qt\5.5\msvc2012
set PATH=%QTDIR%\bin;%PATH%
moc -I�h%QTDIR%\include�h -I�h%QTDIR%\include\QtCore�h -I�h%QTDIR%\include\QtGui�h -o%OUTPUTFILE% %HEADERFILE%
pause;
