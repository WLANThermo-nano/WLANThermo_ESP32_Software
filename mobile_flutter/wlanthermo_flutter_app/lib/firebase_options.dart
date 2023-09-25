// File generated by FlutterFire CLI.
// ignore_for_file: lines_longer_than_80_chars, avoid_classes_with_only_static_members
import 'package:firebase_core/firebase_core.dart' show FirebaseOptions;
import 'package:flutter/foundation.dart'
    show defaultTargetPlatform, kIsWeb, TargetPlatform;

/// Default [FirebaseOptions] for use with your Firebase apps.
///
/// Example:
/// ```dart
/// import 'firebase_options.dart';
/// // ...
/// await Firebase.initializeApp(
///   options: DefaultFirebaseOptions.currentPlatform,
/// );
/// ```
class DefaultFirebaseOptions {
  static FirebaseOptions get currentPlatform {
    if (kIsWeb) {
      throw UnsupportedError(
        'DefaultFirebaseOptions have not been configured for web - '
        'you can reconfigure this by running the FlutterFire CLI again.',
      );
    }
    switch (defaultTargetPlatform) {
      case TargetPlatform.android:
        return android;
      case TargetPlatform.iOS:
        return ios;
      case TargetPlatform.macOS:
        throw UnsupportedError(
          'DefaultFirebaseOptions have not been configured for macos - '
          'you can reconfigure this by running the FlutterFire CLI again.',
        );
      case TargetPlatform.windows:
        throw UnsupportedError(
          'DefaultFirebaseOptions have not been configured for windows - '
          'you can reconfigure this by running the FlutterFire CLI again.',
        );
      case TargetPlatform.linux:
        throw UnsupportedError(
          'DefaultFirebaseOptions have not been configured for linux - '
          'you can reconfigure this by running the FlutterFire CLI again.',
        );
      default:
        throw UnsupportedError(
          'DefaultFirebaseOptions are not supported for this platform.',
        );
    }
  }

  static const FirebaseOptions android = FirebaseOptions(
    apiKey: 'AIzaSyDtXg6V3yRbtJBAhmAtuEKJvPGxmMKRVMg',
    appId: '1:554853523379:android:1c823a24a904a25e44b5c6',
    messagingSenderId: '554853523379',
    projectId: 'wlanthermo-nano',
    databaseURL: 'https://wlanthermo-nano.firebaseio.com',
    storageBucket: 'wlanthermo-nano.appspot.com',
  );

  static const FirebaseOptions ios = FirebaseOptions(
    apiKey: 'AIzaSyAROqTL61ejMsoBrxCJiWDdH-6qupmA-VM',
    appId: '1:554853523379:ios:f2c957e8bcf5abc344b5c6',
    messagingSenderId: '554853523379',
    projectId: 'wlanthermo-nano',
    databaseURL: 'https://wlanthermo-nano.firebaseio.com',
    storageBucket: 'wlanthermo-nano.appspot.com',
    androidClientId: '554853523379-g4399t5o5mi2od5vht7ub6e8br420vqb.apps.googleusercontent.com',
    iosBundleId: 'com.example.wlanthermoFlutterApp',
  );
}
