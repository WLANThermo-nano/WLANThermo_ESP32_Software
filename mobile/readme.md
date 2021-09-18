## Before you start
install cordova by
```
npm install -g cordova
```
see more in https://cordova.apache.org/

## Run app
```
yarn a
```
or 
```
cordova run android
```

## build apk
```
yarn build-android
```
or 
```
cordova build android
```

## IOS 
basically with following commands, but for sure there are more to do to make it work, like Apple develop account..., etc.
```
cordova platform add ios
cordova run ios
cordova build ios
```

## Create Android app that can be published
If you see garbled in the output message of _keytool_, try the following command in your terminal
```
chcp 936 
```

to create keystore file, run 
```
keytool -genkey -v -keystore android.keystore -alias wlanthermo-key -keyalg RSA -keysize 2048 -validity 10000
```

you can download aab file from Github Actions, put the aab file and keystore file in the same directory and run
```
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore android.keystore app-release.aab wlanthermo-key
```
