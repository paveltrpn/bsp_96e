if WScript.Arguments.Length <> 1 then
	WScript.StdErr.WriteLine("arg1 must be filename to create")
	WScript.Quit()
end if
buildnum = cint(cdbl(date) - 100*365)
buildrev = clng(cdbl(time)*24*60*60/10)
set fso = createobject("scripting.filesystemobject")
set fs = fso.createtextfile(WScript.Arguments(0),true)
fs.WriteLine "// build version, generated "&(date+time)
fs.WriteLine ""
fs.WriteLine "#define _BUILDNUM " & buildnum
fs.WriteLine "#define _BUILDREV " & buildrev
fs.WriteLine "#define _BUILDDATE """& date &""""
fs.WriteLine ""