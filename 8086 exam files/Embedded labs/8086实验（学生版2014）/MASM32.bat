d:\\masm32\bin\ml /Zm /Zd /Zi /Zf /c %1
set str=%1
set str=%str:~0,-4%
d:\\masm32\bin\link16 /CO /NOD /CODEVIEW /stack:256 %str%.obj,%str%.exe,nul.map,,