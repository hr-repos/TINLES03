import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_browser_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'package:flutter/foundation.dart';
import '../models/robot_mapper.dart';
import 'dart:convert';

class MqttService {
  late MqttClient client;
  final RobotMapper mapper;

  MqttService(this.mapper);

  Future<void> connect() async {
    const clientId = 'flutter_client';
    final wsUrl = 'ws://192.168.11.156';
    final tcpUrl = '192.168.4.1';

    if (kIsWeb) {
      client = MqttBrowserClient.withPort(wsUrl, clientId, 9001);
    } else {
      client = MqttServerClient(tcpUrl, clientId);
    }
    client.logging(on: true);

    client.onConnected = onConnected;
    client.onDisconnected = onDisconnected;

    final connMessage = MqttConnectMessage()
        .withClientIdentifier(clientId)
        .startClean()
        .withWillQos(MqttQos.atLeastOnce);

    client.connectionMessage = connMessage;

    try {
      await client.connect();
    } catch (e) {
      print("MQTT connection to port 9001 failed: $e");
    }
  }

  void onConnected() {
    client.subscribe('sensors/recalculated', MqttQos.atMostOnce);
    client.subscribe('robot/position', MqttQos.atMostOnce);
    client.updates!.listen((List<MqttReceivedMessage<MqttMessage>> messages) {
      for (var message in messages) {
        _processMessage(message);
      }
    });
  }

  void _processMessage(MqttReceivedMessage<MqttMessage> message) {
    final payload = message.payload as MqttPublishMessage;
    final jsonString =
        MqttPublishPayload.bytesToStringAsString(payload.payload.message);

    try {
      final jsonData = jsonDecode(jsonString);
      if (message.topic == 'sensors/recalculated') {
        mapper.processSensorData(jsonData);
      } else if (message.topic == 'robot/position') {
        final x = jsonData['x']?.toDouble() ?? 0.0;
        final y = jsonData['y']?.toDouble() ?? 0.0;
        mapper.updatePosition(x, y);
      }
    } catch (e) {
      print("Error processing message: $e");
    }
  }

  void onDisconnected() {
    print('Disconnected from MQTT broker');
  }

  void sendCommand(String command) {
    final builder = MqttClientPayloadBuilder();
    builder.addString(command);
    client.publishMessage(
        'robot/command', MqttQos.atLeastOnce, builder.payload!);
    print('Sent command: $command');
  }

  void disconnect() {
    client.disconnect();
  }
}
