set OUTPUTFILE=StateMachineViewMock.cpp
set HEADERFILE=StateMachineView.h
set QTDIR=C:\Qt\4.8.5
set PATH=%QTDIR%\bin;%PATH%
moc -IÅh%QTDIR%\includeÅh -IÅh%QTDIR%\include\QtCoreÅh -IÅh%QTDIR%\include\QtGuiÅh -o%OUTPUTFILE% %HEADERFILE%
pause;
