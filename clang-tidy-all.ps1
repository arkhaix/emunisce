pushd .
cmd.exe /c 'bazel clean'
Get-ChildItem -Path . -Directory -Recurse |
    foreach {
        cd $_.FullName
        cmd.exe /c 'clang-tidy *.cpp --quiet -fix -checks="readability-braces-around-statements" -- -I. -I.. -I..\.. -I..\..\.. -I..\Gameboy -I..\..\Gameboy -I..\Machine -I..\..\Machine -I..\Platform -I..\..\Platform -I..\Utility -I..\..\Utility -I..\BaseApplication -I..\..\BaseApplication -I..\WindowsApplication -I..\..\WindowsApplication'
    }
popd