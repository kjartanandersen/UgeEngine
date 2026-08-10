@echo off
pushd ..\
call msbuild Uge.sln /p:Configuration=Debug /p:Platform=x64 /m
popd