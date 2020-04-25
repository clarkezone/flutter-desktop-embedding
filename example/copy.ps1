$sourcepath = '..\..\engine\src\out\host_debug_unopt\'
#$sourcepath = 'C:\src\f\engine\src\out\host_debug\'

$fdll = Join-Path -Path $sourcepath -ChildPath 'flutter_windows.dll'
$fpdb = Join-Path -Path $sourcepath -ChildPath 'flutter_windows.dll.pdb'
$fexp = Join-Path -Path $sourcepath -ChildPath 'flutter_windows.dll.exp'
$flib = Join-Path -Path $sourcepath -ChildPath 'flutter_windows.dll.lib'
$cwrapper = Join-Path -Path $sourcepath -ChildPath 'cpp_client_wrapper'
$flutterwindowsh = Join-Path -Path $sourcepath -ChildPath 'flutter_windows.h'
$flutterexport = Join-Path -Path $sourcepath -ChildPath 'flutter_export.h'
$fluttermessenger = Join-Path -Path $sourcepath -ChildPath 'flutter_messenger.h'
$flutterpluginregistar = Join-Path -Path $sourcepath -ChildPath 'flutter_plugin_registrar.h'

# win32
copy-item $fdll .\windows\flutter\ephemeral\.
copy-item $fpdb .\windows\flutter\ephemeral\.
copy-item $fexp .\windows\flutter\ephemeral\.
copy-item $flib .\windows\flutter\ephemeral\.

# win32 runner
#copy-item $fdll .\build\windows\x64\Debug\Runner\.
#copy-item $fpdb .\build\windows\x64\Debug\Runner\.

# uwp
copy-item $flib .\windowsuwp\flutter\ephemeral\.
copy-item $fdll .\windowsuwp\flutter\ephemeral\.
copy-item $fpdb .\windowsuwp\flutter\ephemeral\.
copy-item $fexp .\windowsuwp\flutter\ephemeral\.

# uwp package
copy-item $fdll .\windowsuwp\.
copy-item $fpdb .\windowsuwp\.

# uwp runner
copy-item $fdll .\windowsuwp\x64\Debug\windowsuwp\AppX\.
copy-item $fpdb .\windowsuwp\x64\Debug\windowsuwp\AppX\.
#copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.exp .\windowsuwp\x64\Debug\windowsuwp\AppX\.

#copy the data into UWP
# Copy-Item -Recurse -Force .\build\windows\x64\Debug\Runner\data .\windowsuwp\x64\Debug\windowsuwp\AppX\Assets\.

#source
Copy-Item -Recurse -Force $cwrapper .\windows\flutter\ephemeral\.
copy-item $flutterwindowsh .\windows\flutter\ephemeral\.
Copy-Item -Recurse -Force $cwrapper .\windowsuwp\flutter\ephemeral\.
copy-item $flutterwindowsh .\windowsuwp\flutter\ephemeral\.
copy-item $flutterexport .\windowsuwp\flutter\ephemeral\.
copy-item $fluttermessenger .\windowsuwp\flutter\ephemeral\.
copy-item $flutterpluginregistar .\windowsuwp\flutter\ephemeral\.

