copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll .\windows\flutter\ephemeral\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.pdb .\windows\flutter\ephemeral\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.exp .\windows\flutter\ephemeral\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.lib .\windows\flutter\ephemeral\.

copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll .\build\windows\x64\Debug\Runner\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.pdb .\build\windows\x64\Debug\Runner\.

#copy the data into UWP
Copy-Item -Recurse -Force .\build\windows\x64\Debug\Runner\data .\windowsuwp\x64\Debug\windowsuwp\AppX\Assets\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll .\windowsuwp\x64\Debug\windowsuwp\AppX\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.pdb .\windowsuwp\x64\Debug\windowsuwp\AppX\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.exp .\windowsuwp\x64\Debug\windowsuwp\AppX\.

copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.lib .\windowsuwp\flutter\ephemeral\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll .\windowsuwp\flutter\ephemeral\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.pdb .\windowsuwp\flutter\ephemeral\.
copy-item ..\..\engine\src\out\host_debug_unopt\flutter_windows.dll.exp .\windowsuwp\flutter\ephemeral\.