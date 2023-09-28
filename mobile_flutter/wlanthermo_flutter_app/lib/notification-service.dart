import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';

class NotificationService {
  static Future initialize(FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin) async {
    var androidInitialize = new AndroidInitializationSettings('mipmap/ic_launcher');
    var iosInitialize = new DarwinInitializationSettings();
    var initializationSettings = new InitializationSettings(android: androidInitialize, iOS: iosInitialize);

    await flutterLocalNotificationsPlugin.initialize(initializationSettings);

    FirebaseMessaging.onMessage.listen((message) {
      NotificationService.showBigTextNotification(
          title: message.notification?.title ?? "",
          body: message.notification?.body ?? "",
          flutterLocalNotificationsPlugin: flutterLocalNotificationsPlugin
      );
    });
  }

  static Future showBigTextNotification({
    var id = 0,
    required String title,
    required String body,
    var payload, required FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin}) async {
    AndroidNotificationDetails androidNotificationDetails =
      const AndroidNotificationDetails(
          'wlanthermoChannel_id',
          'wlanthermoChannel_name',
           playSound: true,
            importance: Importance.high,
            priority: Priority.high);

    // TODO: ios
    var notification = NotificationDetails(
        android: androidNotificationDetails,
        iOS: const DarwinNotificationDetails());
    await flutterLocalNotificationsPlugin.show(0, title, body, notification);
  }
}