$nextionEditorUrl = "https://nextion.tech/download/nextion-setup-v1-60-2.zip"
$nextionEditorZip = $PSScriptRoot + "\nextion.zip"
$nextionEditorFolder = $PSScriptRoot + "\nextion"
$nextionEditor = $PSScriptRoot + "\nextion\NextionEditor.exe"
$nextionHmiFile = $PSScriptRoot + "\miniVx.HMI"
$nextionTftFile = $PSScriptRoot + "\miniVx.tft"
$nextionArtifactsFolder = $PSScriptRoot + "\artifacts"
$nextionDataFolder = $PSScriptRoot + "\data"
$javaBin = "C:\Program Files\Java\zulu-11-azure-jdk_11.33.15-11.0.4-win_x64\bin\java.exe"
#$javaBin = "C:\Program Files\Java\jdk-13.0.1\bin\java.exe"
$sikulixUrl = "https://launchpad.net/sikuli/sikulix/2.0.2/+download/sikulix-2.0.2.jar"
$sikulix = $PSScriptRoot + "\sikulix-2.0.2.jar"
$sikulixScript = $PSScriptRoot + "\nextion.sikuli"
$jythonUrl = "https://repo1.maven.org/maven2/org/python/jython-standalone/2.7.1/jython-standalone-2.7.1.jar"
$jython = $PSScriptRoot + "\jython-standalone-2.7.1.jar"

(New-Object System.Net.WebClient).DownloadFile($nextionEditorUrl, $nextionEditorZip)
(New-Object System.Net.WebClient).DownloadFile($sikulixUrl, $sikulix)
(New-Object System.Net.WebClient).DownloadFile($jythonUrl, $jython)
Expand-Archive -LiteralPath $nextionEditorZip -DestinationPath $nextionEditorFolder -Force
Start-Process -FilePath $nextionEditor -ArgumentList $nextionHmiFile
#Start-Sleep -s 10

New-Item -ItemType Directory -Force -Path $nextionArtifactsFolder
New-Item -ItemType Directory -Force -Path $nextionDataFolder
Remove-Item "$nextionArtifactsFolder\*.tft"
Remove-Item "*.tft"
Remove-Item "$nextionDataFolder\*.tft.zlib"

$nextionDirection = "0"
$nextionSeries = "Enhanced"
$nextionModel = "NX3224K028"
Start-Process -FilePath $javaBin -ArgumentList "-jar $sikulix -d -r $sikulixScript --args $nextionDirection $nextionSeries $nextionModel" -Wait -NoNewWindow
if(!((Format-Hex -Path $nextionTftFile -Offset 0x2e -Count 4) -like '*2E   9D DC AB BD*')) {exit 1}
if(!((Format-Hex -Path $nextionTftFile -Offset 0x14 -Count 1) -like '*14   01*')) {exit 1}
Move-Item -Path $nextionTftFile -Destination "$nextionArtifactsFolder\$nextionModel-$nextionDirection.tft"

$nextionDirection = "180"
$nextionSeries = "Enhanced"
$nextionModel = "NX3224K028"
Start-Process -FilePath $javaBin -ArgumentList "-jar $sikulix -d -r $sikulixScript --args $nextionDirection $nextionSeries $nextionModel" -Wait -NoNewWindow
if(!((Format-Hex -Path $nextionTftFile -Offset 0x2e -Count 4) -like '*2E   9D DC AB BD*')) {exit 1}
if(!((Format-Hex -Path $nextionTftFile -Offset 0x14 -Count 1) -like '*14   03*')) {exit 1}
Move-Item -Path $nextionTftFile -Destination "$nextionArtifactsFolder\$nextionModel-$nextionDirection.tft"

$nextionDirection = "0"
$nextionSeries = "Enhanced"
$nextionModel = "NX3224K024"
Start-Process -FilePath $javaBin -ArgumentList "-jar $sikulix -d -r $sikulixScript --args $nextionDirection $nextionSeries $nextionModel" -Wait -NoNewWindow
if(!((Format-Hex -Path $nextionTftFile -Offset 0x2e -Count 4) -like '*2E   01 93 7E 5C*')) {exit 1}
if(!((Format-Hex -Path $nextionTftFile -Offset 0x14 -Count 1) -like '*14   01*')) {exit 1}
Move-Item -Path $nextionTftFile -Destination "$nextionArtifactsFolder\$nextionModel-$nextionDirection.tft"

$nextionDirection = "180"
$nextionSeries = "Enhanced"
$nextionModel = "NX3224K024"
Start-Process -FilePath $javaBin -ArgumentList "-jar $sikulix -d -r $sikulixScript --args $nextionDirection $nextionSeries $nextionModel" -Wait -NoNewWindow
if(!((Format-Hex -Path $nextionTftFile -Offset 0x2e -Count 4) -like '*2E   01 93 7E 5C*')) {exit 1}
if(!((Format-Hex -Path $nextionTftFile -Offset 0x14 -Count 1) -like '*14   03*')) {exit 1}
Move-Item -Path $nextionTftFile -Destination "$nextionArtifactsFolder\$nextionModel-$nextionDirection.tft"

$nextionDirection = "0"
$nextionSeries = "Basic"
$nextionModel = "NX3224T028"
Start-Process -FilePath $javaBin -ArgumentList "-jar $sikulix -d -r $sikulixScript --args $nextionDirection $nextionSeries $nextionModel" -Wait -NoNewWindow
if(!((Format-Hex -Path $nextionTftFile -Offset 0x2e -Count 4) -like '*2E   3B 38 43 14*')) {exit 1}
if(!((Format-Hex -Path $nextionTftFile -Offset 0x14 -Count 1) -like '*14   01*')) {exit 1}
Move-Item -Path $nextionTftFile -Destination "$nextionArtifactsFolder\$nextionModel-$nextionDirection.tft"

$nextionDirection = "180"
$nextionSeries = "Basic"
$nextionModel = "NX3224T028"
Start-Process -FilePath $javaBin -ArgumentList "-jar $sikulix -d -r $sikulixScript --args $nextionDirection $nextionSeries $nextionModel" -Wait -NoNewWindow
if(!((Format-Hex -Path $nextionTftFile -Offset 0x2e -Count 4) -like '*2E   3B 38 43 14*')) {exit 1}
if(!((Format-Hex -Path $nextionTftFile -Offset 0x14 -Count 1) -like '*14   03*')) {exit 1}
Move-Item -Path $nextionTftFile -Destination "$nextionArtifactsFolder\$nextionModel-$nextionDirection.tft"

cd "$PSScriptRoot\nextion.spiffs"
python nextion_spiffs.py
