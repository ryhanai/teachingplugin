set OUTPUTFILE=StateMachineViewMock.cpp
set HEADERFILE=StateMachineView.h
set QTDIR=C:\Qt\4.8.5
set PATH=%QTDIR%\bin;%PATH%
moc -I�h%QTDIR%\include�h -I�h%QTDIR%\include\QtCore�h -I�h%QTDIR%\include\QtGui�h -o%OUTPUTFILE% %HEADERFILE%
pause;
