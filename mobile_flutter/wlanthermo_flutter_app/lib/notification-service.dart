import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';

const bellSoundChannelId = "wlanthermo_channel_bell_id";
const defaultSoundChannelId = "wlanthermo_channel_default_id";

const bellSoundChannelName = "bell sound notification channel";
const defaultSoundChannelName = "default sound notification channel";

class NotificationService {
  static Future initialize(
      FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin) async {
    var androidInitialize =
        const AndroidInitializationSettings('mipmap/launcher_icon');
    var iosInitialize = const DarwinInitializationSettings();
    var initializationSettings = InitializationSettings(
        android: androidInitialize, iOS: iosInitialize);

    await flutterLocalNotificationsPlugin.initialize(initializationSettings);

    await FirebaseMessaging.instance
        .setForegroundNotificationPresentationOptions(
      alert: true, // Required to display a heads up notification
      badge: true,
      sound: true,
    );

    // create channel for bell sound
    await flutterLocalNotificationsPlugin
          .resolvePlatformSpecificImplementation<AndroidFlutterLocalNotificationsPlugin>()?.
            createNotificationChannel(
        const AndroidNotificationChannel(
          bellSoundChannelId,
          bellSoundChannelName,
          sound: RawResourceAndroidNotificationSound("bell"),
          importance: Importance.high,
          playSound: true
    ));

    // create channel for default sound
    await flutterLocalNotificationsPlugin
        .resolvePlatformSpecificImplementation<AndroidFlutterLocalNotificationsPlugin>()?.
    createNotificationChannel(
        const AndroidNotificationChannel(
            defaultSoundChannelId,
            defaultSoundChannelName,
            importance: Importance.high,
            playSound: true
        ));

    FirebaseMessaging.onMessage.listen((message) {
      NotificationService.showBigTextNotification(
          title: message.notification?.title ?? "",
          body: message.notification?.body ?? "",
          sound: message.data['sound'] ?? "default",
          flutterLocalNotificationsPlugin: flutterLocalNotificationsPlugin);
    });

  }

  static Future showBigTextNotification(
      {var id = 0,
      required String title,
      required String body,
      required String sound,
      var payload,
      required FlutterLocalNotificationsPlugin
          flutterLocalNotificationsPlugin}) async {

    AndroidNotificationDetails androidNotificationDetails;
    if (sound == "bell.mp3") {
      androidNotificationDetails =
          const AndroidNotificationDetails(
              bellSoundChannelId, bellSoundChannelName,
              playSound: true,
              sound: RawResourceAndroidNotificationSound("bell"),
              importance: Importance.high,
              priority: Priority.high);
    } else {
      androidNotificationDetails =
          const AndroidNotificationDetails(
              defaultSoundChannelId, defaultSoundChannelName,
              playSound: true,
              importance: Importance.high,
              priority: Priority.high);
    }

    var iosNotificationDetails = DarwinNotificationDetails(
      presentSound: true,
      sound: sound == "bell.mp3" ? "bell.mp3" : null,
    );

    // TODO: ios
    var notification = NotificationDetails(
        android: androidNotificationDetails, iOS: iosNotificationDetails);
    await flutterLocalNotificationsPlugin.show(0, title, body, notification);
  }
}
