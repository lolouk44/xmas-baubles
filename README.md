# xmas-baubles

This project adds an easy way to control LED Xmas Smart Baubles from [Home Assistant](https://home-assistant.io/), an amazing, extensible, open-source home automation system.

I followed Gosse Adema's [instructable](http://www.instructables.com/id/Illuminated-Christmas-Tree-Ornament-WiFi-Controlle/) to create the baubles and his origical code but wanted to change the effects and wanted to be able to control the baubles from Home Assistant, so I adapted the code and took extracts from corbanmailloux's [esp-mqtt-rgb-led project](https://github.com/corbanmailloux/esp-mqtt-rgb-led).
As of version 0.26, the [MQTT JSON light platform](https://home-assistant.io/components/light.mqtt_json/) has been merged into Home Assistant.

By sending a JSON payload (in an MQTT message), Home Assistant can include whichever fields are necessary, reducing the round trips from 3 to 1. For example, this is a sample payload including most of the fields:
```json
{
  "state": "ON",
  "effect": "sparkle",
}
```

## Installation/Configuration

To set this system up, you need to configure the [MQTT JSON light](https://home-assistant.io/components/light.mqtt_json/) component in Home Assistant and set up a light to control. This guide assumes that you already have Home Assistant set up and running. If not, see the installation guides [here](https://home-assistant.io/getting-started/).

### The Home Assistant Side
1. In your `configuration.yaml`, add the following:

    ```yaml
    light:
      - platform: mqtt_json
        name: mqtt_json_light_1
        state_topic: "home/json_brightness"
        command_topic: "home/json_brightness/set"
        brightness: false
        effect: true
        effect_list: [sparkle, rainbow, uniglow]
        optimistic: false
        qos: 0


    ```
2. Set the `name`, `state_topic`, and `command_topic` to values that make sense for you.
3. Restart Home Assistant. Depending on how you installed it, the process differs. For a Raspberry Pi All-in-One install, use `sudo systemctl restart home-assistant.service` (or just restart the Pi).

### The Light Side

1. Using the Library Manager in the Arduino IDE, install [ArduinoJSON](https://github.com/bblanchon/ArduinoJson/), [PubSubClient](http://pubsubclient.knolleary.net/), and [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel). You can find the Library Manager in the "Sketch" menu under "Include Library" -> "Manage Libraries..."
2. Update the `config-sample.h` file with your settings for pin numbers, WiFi settings, and MQTT settings.
3. Ensure that the `CONFIG_MQTT_CLIENT_ID` setting is a unique value for your network.
4. Set `CONFIG_MQTT_TOPIC_STATE` and `CONFIG_MQTT_TOPIC_SET` to match the values you put in your `configuration.yaml`.
5. Save the configuration file as `config.h`.
6. Open the `.ino` file in the Arduino IDE and upload to a "WeMos D1 R1 & Mini" with the correct connections.