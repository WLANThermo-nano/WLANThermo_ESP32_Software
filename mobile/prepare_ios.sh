#!/bin/bash

rm -rf node_modules
rm -rf platforms
rm -rf plugins

gpg --quiet --batch --yes --decrypt --passphrase="${SECRET}" --output GoogleService-Info.plist ../.github/secrets/GoogleService-Info.plist.gpg
gpg --quiet --batch --yes --decrypt --passphrase="${SECRET}" --output google-services.json ../.github/secrets/google-services.json.gpg
gpg --quiet --batch --yes --decrypt --passphrase="${SECRET}" --output resources/bell.mp3 ../.github/secrets/bell.mp3.gpg

cd ../webui
npm install
npm run build-mobile
cd ../mobile
npm install -g cordova cordova-res
cordova platform add ios@latest
npm install
cordova-res
