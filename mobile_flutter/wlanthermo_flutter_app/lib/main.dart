import 'dart:async' show Future;
import 'dart:convert';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:shared_preferences/shared_preferences.dart';

main() async {
  WidgetsFlutterBinding.ensureInitialized();
  String htmlString = await rootBundle.loadString('assets/html/index.html');
 
  String localIp = '';
  final List<String> privateNetworkMasks = ['10', '172.16', '192.168'];
  for (var interface in await NetworkInterface.list()) {
      for (var addr in interface.addresses) {
        for (final possibleMask in privateNetworkMasks) {
          if (addr.address.startsWith(possibleMask)) {
            localIp = addr.address;
            break;
          }
        }
      }
    }

  runApp(MyApp(htmlString, localIp));
}

class MyApp extends StatelessWidget {
  final String htmlString;
  final String localIp;

  const MyApp(this.htmlString, this.localIp, {super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        title: 'Flutter Demo',
        home: Scaffold(
          // appBar: AppBar(
          //   title: Text("web view")
          // ),
          body: InAppWebView(
            initialUrlRequest: URLRequest(
                url: Uri.dataFromString(htmlString,
                    mimeType: 'text/html',
                    encoding: Encoding.getByName('utf-8'))),
            onLoadStop: (controller, url) {
              controller.addJavaScriptHandler(
                  handlerName: 'getIpAddress',
                  callback: (args) {
                    // return data to the JavaScript side!
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
              controller.addJavaScriptHandler(handlerName: 'handlerFooWithArgs', callback: (args) {
                    print(args);
                    // it will print: [1, true, [bar, 5], {foo: baz}, {bar: bar_value, baz: baz_value}]
                  });
            },
            
          ),
        ));
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;

  void _incrementCounter() {
    setState(() {
      // This call to setState tells the Flutter framework that something has
      // changed in this State, which causes it to rerun the build method below
      // so that the display can reflect the updated values. If we changed
      // _counter without calling setState(), then the build method would not be
      // called again, and so nothing would appear to happen.
      _counter++;
    });
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
      appBar: AppBar(
        // TRY THIS: Try changing the color here to a specific color (to
        // Colors.amber, perhaps?) and trigger a hot reload to see the AppBar
        // change color while the other colors stay the same.
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        // Here we take the value from the MyHomePage object that was created by
        // the App.build method, and use it to set our appbar title.
        title: Text(widget.title),
      ),
      body: Center(
        // Center is a layout widget. It takes a single child and positions it
        // in the middle of the parent.
        child: Column(
          // Column is also a layout widget. It takes a list of children and
          // arranges them vertically. By default, it sizes itself to fit its
          // children horizontally, and tries to be as tall as its parent.
          //
          // Column has various properties to control how it sizes itself and
          // how it positions its children. Here we use mainAxisAlignment to
          // center the children vertically; the main axis here is the vertical
          // axis because Columns are vertical (the cross axis would be
          // horizontal).
          //
          // TRY THIS: Invoke "debug painting" (choose the "Toggle Debug Paint"
          // action in the IDE, or press "p" in the console), to see the
          // wireframe for each widget.
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'You have pushed the button this many times:',
            ),
            Text(
              '$_counter',
              style: Theme.of(context).textTheme.headlineMedium,
            ),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _incrementCounter,
        tooltip: 'Increment',
        child: const Icon(Icons.add),
      ), // This trailing comma makes auto-formatting nicer for build methods.
    );
  }
}
