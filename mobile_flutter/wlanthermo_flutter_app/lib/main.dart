import 'dart:async' show Future;
import 'dart:convert';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'firebase_options.dart';
import 'package:device_info_plus/device_info_plus.dart';
import 'notification-service.dart';

final FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin = FlutterLocalNotificationsPlugin();

late FirebaseMessaging messaging;

main() async {
  SystemChrome.setSystemUIOverlayStyle(
    const SystemUiOverlayStyle(
      statusBarColor: Colors.black,
    ),
  );

  WidgetsFlutterBinding.ensureInitialized();
  String htmlString = await rootBundle.loadString('assets/html/index.html');
  
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );

  messaging = FirebaseMessaging.instance;

  FirebaseMessaging.instance.onTokenRefresh
    .listen((fcmToken) {
      // TODO: If necessary send token to application server.

      // Note: This callback is fired at each app startup and whenever a new
      // token is generated.
    })
    .onError((err) {
      // Error getting token.
    });

  runApp(MyApp(htmlString));
}

class MyApp extends StatelessWidget {
  final String htmlString;

  const MyApp(this.htmlString, {super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {    
    return MaterialApp(
        home: MyHomePage(title: 'Wlanthermo', htmlString: htmlString)
        );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title, required this.htmlString});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;
  final String htmlString;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {

  @override
  void initState() {
    super.initState();
    NotificationService.initialize(flutterLocalNotificationsPlugin);
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    return Scaffold(
      body: SafeArea(
        child:         InAppWebView(
          initialUrlRequest: URLRequest(
              url: Uri.dataFromString(widget.htmlString,
                  mimeType: 'text/html',
                  encoding: Encoding.getByName('utf-8'))),
          onLoadStop: (controller, url) {
            controller.addJavaScriptHandler(
                handlerName: 'debug',
                callback: (args) async {

                  return {'value': 'ok'};
                });
            controller.addJavaScriptHandler(
                handlerName: 'getIpAddress',
                callback: (args) async {
                  // Getting local ip which matches the private network pattern.
                  String localIp = '';
                  final List<String> privateNetworkMasks = ['192.168', '10', '172.16'];
                  for (var interface in await NetworkInterface.list()) {
                    if (interface.name.startsWith("wlan")) {
                      for (var addr in interface.addresses) {
                        for (final possibleMask in privateNetworkMasks) {
                          if (addr.address.startsWith(possibleMask)) {
                            localIp = addr.address;
                            break;
                          }
                        }
                      }
                    }
                  }
                  return {'localIp': localIp};
                });
            controller.addJavaScriptHandler(
                handlerName: 'saveData',
                // args: ['key', 'value']
                callback: (args) async {
                  final prefs = await SharedPreferences.getInstance();
                  await prefs.setString(args[0], args[1]);

                  return {'message': 'ok'};
                });
            controller.addJavaScriptHandler(
                handlerName: 'getData',
                // args: ['key']
                callback: (args) async {
                  final prefs = await SharedPreferences.getInstance();
                  var value = await prefs.getString(args[0]);

                  return {'value': value};
                });
            controller.addJavaScriptHandler(
                handlerName: 'getFCMToken',
                callback: (args) async {
                  final fcmToken = await FirebaseMessaging.instance.getToken();
                  print(fcmToken);
                  return {'token': fcmToken};
                });
            controller.addJavaScriptHandler(
                handlerName: 'getDeviceModel',
                callback: (args) async {
                  DeviceInfoPlugin deviceInfo = DeviceInfoPlugin();
                  String model = '';
                  if (Platform.isAndroid) {
                    AndroidDeviceInfo androidInfo = await deviceInfo.androidInfo;
                    model = androidInfo.model;
                  } else if (Platform.isIOS) {
                    IosDeviceInfo iosInfo = await deviceInfo.iosInfo;
                    model = iosInfo.model;
                  }
                  return {'model': model};
                });
          },
        ),
      ),

    );
  }
}
