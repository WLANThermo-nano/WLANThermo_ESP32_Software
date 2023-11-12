import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'package:bonsoir/bonsoir.dart';
import 'package:device_info_plus/device_info_plus.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:flutter_native_splash/flutter_native_splash.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:url_launcher/url_launcher.dart';

import 'firebase_options.dart';

final FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin = FlutterLocalNotificationsPlugin();

late FirebaseMessaging messaging;

Future main() async {
  SystemChrome.setSystemUIOverlayStyle(
    const SystemUiOverlayStyle(
      statusBarColor: Colors.black,
    ),
  );

  WidgetsBinding widgetsBinding = WidgetsFlutterBinding.ensureInitialized();
  FlutterNativeSplash.preserve(widgetsBinding: widgetsBinding);

  if (Platform.isAndroid) {
    await AndroidInAppWebViewController.setWebContentsDebuggingEnabled(true);
  }

  String htmlString = await rootBundle.loadString('assets/html/index.html');

  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );

  runApp(MaterialApp(
      home: MyApp(htmlString: htmlString)
  ));
}

class MyApp extends StatefulWidget {
  final String htmlString;

  const MyApp({required this.htmlString});

  @override
  _MyAppState createState() => new _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final GlobalKey webViewKey = GlobalKey();

  late InAppWebViewController webViewController;
  InAppWebViewGroupOptions options = InAppWebViewGroupOptions(
      crossPlatform: InAppWebViewOptions(
        useShouldOverrideUrlLoading: true,
        mediaPlaybackRequiresUserGesture: false,
      ),
      android: AndroidInAppWebViewOptions(
        useHybridComposition: true,
      ),
      ios: IOSInAppWebViewOptions(
        allowsInlineMediaPlayback: true,
      ));

  late PullToRefreshController pullToRefreshController;
  final urlController = TextEditingController();

  @override
  void initState() {
    super.initState();

    pullToRefreshController = PullToRefreshController(
      options: PullToRefreshOptions(
        color: Colors.blue,
      ),
      onRefresh: () async {
        if (Platform.isAndroid) {
          webViewController?.reload();
        } else if (Platform.isIOS) {
          webViewController?.loadUrl(
              urlRequest: URLRequest(url: await webViewController?.getUrl()));
        }
      },
    );
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return WillPopScope(
      onWillPop: () async {
        if (webViewController != null) {
          if (await webViewController.canGoBack()) {
            var webHistory = await webViewController.getCopyBackForwardList();
            if (webHistory?.currentIndex != null && webHistory!.currentIndex! <= 1) {
              return true;
            }
            webViewController?.goBack();
          }
          return false;
        }
        return true;
      },
      child: Scaffold(
          body: SafeArea(
              child: Column(children: <Widget>[
                Expanded(
                  child: Stack(
                    children: [
                      InAppWebView(
                        key: webViewKey,
                        initialUrlRequest:
                        URLRequest(
                            url: Uri.dataFromString(widget.htmlString,
                                mimeType: 'text/html',
                                encoding: Encoding.getByName('utf-8'))),
                        initialOptions: options,
                        pullToRefreshController: pullToRefreshController,
                        onWebViewCreated: (controller) {
                          webViewController = controller;
                        },
                        androidOnPermissionRequest: (controller, origin, resources) async {
                          return PermissionRequestResponse(
                              resources: resources,
                              action: PermissionRequestResponseAction.GRANT);
                        },
                        onLoadStop: (controller, url) {
                          FlutterNativeSplash.remove();
                          controller.addJavaScriptHandler(
                              handlerName: 'debug',
                              callback: (args) async {

                                return {'value': 'ok'};
                              });
                          controller.addJavaScriptHandler(
                              handlerName: 'getNW',
                              callback: (args) async {
                                var rs = "";
                                for (var interface in await NetworkInterface.list()) {
                                  // wlan means wifi in android.
                                  rs += "${interface.name}: ";

                                  for (var addr in interface.addresses) {
                                    rs += interface.addresses.join(", ");
                                  }
                                }

                                return {'value': rs};
                              });
                          controller.addJavaScriptHandler(
                              handlerName: 'scanByZeroConfig',
                              callback: (args) async {
                                // This is the type of service we're looking for :
                                String type = '_wlanthermo._tcp';
                                // Once defined, we can start the discovery :
                                BonsoirDiscovery discovery = BonsoirDiscovery(type: type);
                                await discovery.ready;

                                // If you want to listen to the discovery :
                                discovery.eventStream!.listen((event) async { // `eventStream` is not null as the discovery instance is "ready" !
                                  if (event.type == BonsoirDiscoveryEventType.discoveryServiceFound) {
                                    print('Service found : ${event.service?.toJson()}');
                                    event.service!.resolve(discovery.serviceResolver); // Should be called when the user wants to connect to this service.
                                  } else if (event.type == BonsoirDiscoveryEventType.discoveryServiceResolved) {
                                    var service = event.service as ResolvedBonsoirService;

                                    // dispatch event to the web app
                                    await controller.evaluateJavascript(source: """
                            window.dispatchEvent(new CustomEvent("serviceResolved", {
                              detail: {
                                name: "${service.name}",
                                ip: "${service.ip}"
                              }
                            }));
                          """);
                                  } else if (event.type == BonsoirDiscoveryEventType.discoveryServiceLost) {
                                    print('Service lost : ${event.service?.toJson()}');
                                  }
                                });

                                // Start discovery **after** having listened to discovery events
                                await discovery.start();

                                return {'value': 'ok'};
                              });
                          controller.addJavaScriptHandler(
                              handlerName: 'getIpAddress',
                              callback: (args) async {
                                // Getting local ip which matches the private network pattern.
                                String localIp = '';
                                final List<String> privateNetworkMasks = ['192.168', '10', '172.16'];
                                for (var interface in await NetworkInterface.list()) {
                                  // wlan means wifi in android.
                                  if (interface.name.startsWith("wlan") || Platform.isIOS) {
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
                                print("token: $fcmToken");
                                return {'token': fcmToken};
                              });
                          controller.addJavaScriptHandler(
                              handlerName: 'openExternalLink',
                              callback: (args) async {
                                final Uri url = Uri.parse(args[0]);
                                if (!await launchUrl(url)) {
                                  throw Exception('Could not launch $url');
                                }
                                return {'value': 'ok'};
                              });
                          controller.addJavaScriptHandler(
                              handlerName: 'requestNotificationPermission',
                              callback: (args) async {
                                // it's required by android version 13
                                if (Platform.isAndroid) {
                                  FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin =
                                  FlutterLocalNotificationsPlugin();
                                  flutterLocalNotificationsPlugin.resolvePlatformSpecificImplementation<
                                      AndroidFlutterLocalNotificationsPlugin>()?.requestPermission();
                                }
                                return {'value': 'ok'};
                              });
                          controller.addJavaScriptHandler(
                              handlerName: 'getDeviceInfo',
                              callback: (args) async {
                                DeviceInfoPlugin deviceInfo = DeviceInfoPlugin();
                                String model = '';
                                String id = '';
                                if (Platform.isAndroid) {
                                  AndroidDeviceInfo androidInfo = await deviceInfo.androidInfo;
                                  model = androidInfo.model;
                                  id = androidInfo.id;
                                } else if (Platform.isIOS) {
                                  IosDeviceInfo iosInfo = await deviceInfo.iosInfo;
                                  model = iosInfo.model;
                                  id = iosInfo.identifierForVendor!;
                                }
                                return {'model': model, 'id': id};
                              });
                        },
                        onLoadError: (controller, url, code, message) {
                          pullToRefreshController.endRefreshing();
                        },
                        onUpdateVisitedHistory: (controller, url, androidIsReload) {
                        },
                        onConsoleMessage: (controller, consoleMessage) {
                          print(consoleMessage);
                        },
                      ),
                    ],
                  ),
                ),
              ]))
      ),
    );
  }
}