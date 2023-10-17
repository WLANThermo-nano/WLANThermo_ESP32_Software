import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';

class NotificationService {
  static Future initialize(FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin) async {
    var androidInitialize = new AndroidInitializationSettings('mipmap/launcher_icon');
    var iosInitialize = new DarwinInitializationSettings();
    var initializationSettings = new InitializationSettings(android: androidInitialize, iOS: iosInitialize);

    await flutterLocalNotificationsPlugin.initialize(initializationSettings);

    await FirebaseMessaging.instance.setForegroundNotificationPresentationOptions(
      alert: true, // Required to display a heads up notification
      badge: true,
      sound: true,
    );

    FirebaseMessaging.onMessage.listen((message) {
      NotificationService.showBigTextNotification(
          title: message.notification?.title ?? "",
          body: message.notification?.body ?? "",
          sound: message.data['sound'] ?? "default",
          flutterLocalNotificationsPlugin: flutterLocalNotificationsPlugin
      );
    });
  }

  static Future showBigTextNotification({
    var id = 0,
    required String title,
    required String body,
    required String sound,
    var payload, required FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin}) async {
    AndroidNotificationDetails androidNotificationDetails =
      AndroidNotificationDetails(
          'wlanthermoChannel_id',
          'wlanthermoChannel_name',
           //playSound: true,
           sound: sound == "bell.mp3" ? const RawResourceAndroidNotificationSound("bell") : null,
           importance: Importance.high,
           priority: Priority.high);

    var iosNotificationDetails = DarwinNotificationDetails(
      presentSound: true,
      sound: sound == "bell.mp3" ? "bell.aiff" : null,
    );

    // TODO: ios
    var notification = NotificationDetails(
        android: androidNotificationDetails,
        iOS: iosNotificationDetails);
    await flutterLocalNotificationsPlugin.show(0, title, body, notification);
  }
}