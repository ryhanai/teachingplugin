set OUTPUTFILE=TaskInstanceViewMock5.cpp
set HEADERFILE=TaskInstanceView.h
rem set QTDIR=C:\Qt\4.8.5
set QTDIR=I:\Programs\Qt\5.5\msvc2012
set PATH=%QTDIR%\bin;%PATH%
moc -IÅh%QTDIR%\includeÅh -IÅh%QTDIR%\include\QtCoreÅh -IÅh%QTDIR%\include\QtGuiÅh -o%OUTPUTFILE% %HEADERFILE%
pause;
